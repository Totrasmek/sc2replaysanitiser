# mpq structure

header has offsets for hash table and block table and number of entries in each table
each table entry is 16B
each table is encrypted with a hash key dependent on the type of table with a prebuilt encryption table with 1280 values
hash tables have hashes and point to a block table index
block tables have offsets and sizes

finding a file by names means matching the hash of its name against the entries in the hash table
the entry in the hash table will point to an index in block table
block table has offset and archived size for reading in data

the read data may be broken into sectors. if so, the header helps determine the sector size with sector_size = 512 << header.sector_size_shift 

the first section of the file data is dedicated to the position of the sectors as uint32_t values,

you can use the positions to identify the sectors file data and extract

then you can check if it is compressed, and if so uncompress based on the first bytes of data determining type, zlib or bz2

then message events reader (readers.py) is given the uncompressed data

# replay.message structure

frame?

# plan

- read in file headers and display in a pretty way
- understand how many sections there are etc.
- plan, add a new replay.message section to the end of file? edit original hash and block entries

worried about what the padding between block entries is e.g
```
MPQBlockTableEntry(offset=1232, archived_size=529, size=529, flags=2164261376)
MPQBlockTableEntry(offset=1777, archived_size=999, size=1843, flags=2164261376)
MPQBlockTableEntry(offset=2792, archived_size=13305, size=23288, flags=2164261376)
MPQBlockTableEntry(offset=16113, archived_size=12742, size=23683, flags=2164261376)
MPQBlockTableEntry(offset=38284, archived_size=84, size=84, flags=2164261376)
MPQBlockTableEntry(offset=28971, archived_size=40, size=52, flags=2164261376)
MPQBlockTableEntry(offset=29027, archived_size=537, size=669, flags=2164261376)
MPQBlockTableEntry(offset=29580, archived_size=4828, size=29692, flags=2164261376)
MPQBlockTableEntry(offset=34424, archived_size=817, size=1356, flags=2164261376)
MPQBlockTableEntry(offset=35257, archived_size=435, size=1660, flags=2164261376)
MPQBlockTableEntry(offset=35708, archived_size=12, size=12, flags=2164261376)
MPQBlockTableEntry(offset=35736, archived_size=145, size=239, flags=2164261376)
MPQBlockTableEntry(offset=35897, archived_size=268, size=268, flags=2164261376)
```