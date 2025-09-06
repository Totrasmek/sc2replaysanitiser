import mpyq
import struct
import sys

CHAT_BLOCK_NAME = 'replay.message.events'
COPY_FILE_NAME = 'copy.SC2Replay'

if __name__ == '__main__':
    ######## LOAD ARCHIVE ########
    path = sys.argv[1]
    archive = mpyq.MPQArchive(path)
    print("header")
    print(archive.header)
    print("hash table")
    print(archive.hash_table)
    print("block table")
    print(archive.block_table)
    print("block table entries")
    for entry in archive.block_table:
        print("%6d - %6d" % (entry.offset, entry.offset + entry.archived_size))
    ####### FIND CHAT FILE #######
    chat_hash_entry = archive.get_hash_table_entry(CHAT_BLOCK_NAME)
    assert chat_hash_entry , "%s not in archive" % CHAT_BLOCK_NAME
    chat_block_entry = archive.block_table[chat_hash_entry.block_table_index]
    print("%s block entry" % CHAT_BLOCK_NAME)
    print(chat_block_entry)
    ####### COPY CHAT BLOCK TO END #######
    archive.file.seek(0)
    raw_data = bytearray(archive.file.read())
    copy_chat_block_entry = chat_block_entry
    copy_chat_block_entry.offset = raw_data.size()
    raw_data[archive.header.block_table_offset + chat_hash_entry.block_table_index * struct.calcsize(mpyq.MPQBlockTableEntry.struct_format):
             archive.header.block_table_offset + (chat_hash_entry.block_table_index + 1 ) * struct.calcsize(mpyq.MPQBlockTableEntry.struct_format) - 1] = bytearray(struct.pack(mpyq.MPQBlockTableEntry.struct_format, *copy_chat_block_entry)) # verify
    raw_data.append(raw_data[chat_block_entry.offset:chat_block_entry.offset+chat_block_entry.archived_size]) # verify
    copy_file = open(COPY_FILE_NAME, 'wb')
    copy_file.write(raw_data)
    copy_file.close()