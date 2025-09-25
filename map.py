import mpyq
import sc2reader
from sc2reader.decoders import BitPackedDecoder
from sc2reader.events.message import ChatEvent
import struct
from io import BytesIO
import sys
from dataclasses import dataclass
import bz2
import zlib

CHAT_BLOCK_NAME = 'replay.message.events'
COPY_FILE_NAME = 'copy.SC2Replay'

def _prepare_encryption_table():
    """Prepare encryption table for MPQ hash function."""
    seed = 0x00100001
    crypt_table = {}

    for i in range(256):
        index = i
        for j in range(5):
            seed = (seed * 125 + 3) % 0x2AAAAB
            temp1 = (seed & 0xFFFF) << 0x10

            seed = (seed * 125 + 3) % 0x2AAAAB
            temp2 = (seed & 0xFFFF)

            crypt_table[index] = (temp1 | temp2)

            index += 0x100

    return crypt_table

ENCRYPTION_TABLE = _prepare_encryption_table()

def _hash(string, hash_type):
    """Hash a string using MPQ's hash function."""
    hash_types = {
        'TABLE_OFFSET': 0,
        'HASH_A': 1,
        'HASH_B': 2,
        'TABLE': 3
    }
    seed1 = 0x7FED7FED
    seed2 = 0xEEEEEEEE

    for ch in string.upper():
        if not isinstance(ch, int): ch = ord(ch)
        value = ENCRYPTION_TABLE[(hash_types[hash_type] << 8) + ch]
        seed1 = (value ^ (seed1 + seed2)) & 0xFFFFFFFF
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3 & 0xFFFFFFFF

    return seed1

def _encrypt(data, key):
    """Decrypt hash or block table or a sector."""
    seed1 = key
    seed2 = 0xEEEEEEEE
    result = BytesIO()

    for i in range(len(data) // 4): # 4 bytes for unsigned integer
        seed2 += ENCRYPTION_TABLE[0x400 + (seed1 & 0xFF)]
        seed2 &= 0xFFFFFFFF

        raw_value = struct.unpack("<I", data[i*4:i*4+4])[0]
        value = (raw_value ^ (seed1 + seed2)) & 0xFFFFFFFF

        seed1 = ((~seed1 << 0x15) + 0x11111111) | (seed1 >> 0x0B)
        seed1 &= 0xFFFFFFFF
        seed2 = raw_value + seed2 + (seed2 << 5) + 3 & 0xFFFFFFFF # different to _decrypt

        result.write(struct.pack("<I", value)) # writes a little endian unsigned integer

    return result.getvalue()

def compress(data):
    """Read the compression type and compress file data."""
    compression_type = ord(data[0:1])
    if compression_type == 0:
        return data
    elif compression_type == 2:
        return zlib.compress(data[1:], 15)
    elif compression_type == 16:
        return bz2.compress(data[1:])
    else:
        raise RuntimeError("Unsupported compression type.")

if __name__ == '__main__':
    ######## LOAD ARCHIVE ########
    path = sys.argv[1]
    archive = mpyq.MPQArchive(path, listfile=False)
    ### VERIFY ORIGINAL ARCHIVE ##
    print("#########################################\nOriginal archive messages:")
    replay = sc2reader.load_replay(path)
    for message in replay.messages:
        print(message)
        print(message.player.name)
    print()
    ####### FIND CHAT FILE #######
    chat_hash_entry = archive.get_hash_table_entry(CHAT_BLOCK_NAME)
    assert chat_hash_entry , "%s not in archive" % CHAT_BLOCK_NAME
    chat_block_entry = archive.block_table[chat_hash_entry.block_table_index]
    ### LOAD CHAT BIT OFFSETS ####
    @dataclass
    class Msg:
        chat_event: ChatEvent
        offset_bytes: int # offset from start of chat block to string length stored in 11bits
    msgs: list[Msg] = []
    data=archive.read_file(CHAT_BLOCK_NAME)
    print(data)
    decoder = BitPackedDecoder(data) # bytearray(raw_data[copy_chat_block_entry.offset+archive.header['offset']:])
    while not decoder.done():
        decoder.read_frames()
        pid = decoder.read_bits(5)
        flag = decoder.read_bits(4)
        recipient = decoder.read_bits(3 if replay.base_build >= 21955 else 2)
        if flag == 0: # Client chat message
            string_length=decoder.read_bits(11)
            decoder.byte_align()
            msgs.append(Msg(chat_event=replay.messages[len(msgs)], offset_bytes=decoder.tell()))
            text = decoder.read_aligned_string(string_length)
        elif flag == 1: # Client ping message
            decoder.read_uint32()
            decoder.read_uint32()
        elif flag == 2: # Loading progress message
            decoder.read_uint32()
        decoder.byte_align()
    print("#########################################\Loaded archive messages:")
    for msg in msgs:
        print(msg.chat_event)
        print("Byte offset %s" % msg.offset_bytes)
        print(data[msg.offset_bytes:msg.offset_bytes+len(msg.chat_event.text)])
    ####### EDIT CHAT MSGS #######
    copy_chat_block = bytearray(data[:])
    for msg in msgs:
        copy_chat_block[msg.offset_bytes:msg.offset_bytes+len(msg.chat_event.text)] = ("f"*len(msg.chat_event.text)).encode('utf-8')
    ########## COMPRESS ##########
    if chat_block_entry.flags & mpyq.MPQ_FILE_ENCRYPTED:
        raise NotImplementedError("Encryption is not supported yet.")
    if not chat_block_entry.flags & mpyq.MPQ_FILE_SINGLE_UNIT:
        sector_size = 512 << archive.header['sector_size_shift']
        sectors = chat_block_entry.size // sector_size + 1
        if chat_block_entry.flags & mpyq.MPQ_FILE_SECTOR_CRC:
            crc = True
            sectors += 1
        else:
            crc = False
        unit_offset_store_length = 4*(sectors+1)
        uncompressed_units = copy_chat_block[unit_offset_store_length:]
        compressed_chat_block = bytearray()
        for unit_offset in range(0,len(uncompressed_units),sector_size):
            uncompressed_unit = uncompressed_units[unit_offset:unit_offset+sector_size]
            compressed_unit = compress()
        # section before this is dedicated to storing positions
        # iteratively compress the next part based on sector size
        # add to new positions as you iterate
        for i in range()
    if (chat_block_entry.flags & mpyq.MPQ_FILE_COMPRESS and (chat_block_entry.size > chat_block_entry.archived_size)):
        copy_chat_block = compress(data)
    ### POINT CHAT ENTRY TO END ##
    archive.file.seek(0)
    raw_data = bytearray(archive.file.read())
    copy_chat_block_entry = chat_block_entry._replace(offset=(len(raw_data)-archive.header['offset'])) # edit block entry
    archive.block_table[chat_hash_entry.block_table_index] = copy_chat_block_entry
    copy_block_table = bytearray()
    for entry in archive.block_table: # Convert all entries to structs
        copy_block_table += bytearray(struct.pack(mpyq.MPQBlockTableEntry.struct_format, *entry))
    key = _hash('(block table)', 'TABLE')
    encrypted_copy_block_table = _encrypt(copy_block_table, key)
    ### COPY CHAT BLOCK TO END ###
    block_table_start_index = (
        archive.header["offset"]
        + archive.header["block_table_offset"]
    )
    block_table_end_index = (
        archive.header["offset"]
        + archive.header["block_table_offset"]
        + archive.header["block_table_entries"]*struct.calcsize(mpyq.MPQBlockTableEntry.struct_format)
    )
    raw_data[block_table_start_index:block_table_end_index] = encrypted_copy_block_table
    raw_data += copy_chat_block
    ######### WRITE COPY #########
    copy_file = open(COPY_FILE_NAME, 'wb')
    copy_file.write(raw_data)
    copy_file.close()
    #### VERIFY COPY ARCHIVE #####
    print("#########################################\nCopy archive messages:")
    replay = sc2reader.load_replay(COPY_FILE_NAME)
    for message in replay.messages:
        print(message.text)
    print()
    # TODO understand the message reader frame structure