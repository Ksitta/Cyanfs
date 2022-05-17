CyanFS RocksDB Plugin
------
# Running Method

- Pull the latest rocksdb repo from [github](https://github.com/facebook/rocksdb/)
- Put this cyanfs directory under `rocksdb\plugin`
- Back to the root directory of rocksdb
- Build rocksdb with CyanFS plugin, command as follows:
  ```bash
  ROCKSDB_PLUGINS="cyanfs" make -j4 db_bench install DEBUG_LEVEL=2 2>compile.log
  ```
- Run the testbench
  ```bash
  ./db_bench --benchmarks=fillrandom --fs_uri=cyanfs --compression_type=none
  ```
- Run examples
  - Move to `examples` folder
  - Run `make all`
  - Run the examples as you like

# Result

```bash
$./db_bench --benchmarks=fillrandom --fs_uri=cyanfs --compression_type=none > run.log
RocksDB:    version 7.3
Date:       Tue May 17 16:10:02 2022
CPU:        12 * Intel(R) Core(TM) i7-10750H CPU @ 2.60GHz
CPUCache:   12288 KB
... finished 500000 ops                              
put error: Corruption: Bad table magic number: expected 9863518390377041911, found 0 in
/tmp/rocksdbtest-1000/dbbench/000012.sst
```