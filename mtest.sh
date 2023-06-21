make clean
source ./activate
cd vm
make clean
make
cd build

pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/mmap-clean:mmap-clean -p ../../tests/vm/sample.txt:sample.txt --swap-disk=4 -- -q   -f run mmap-clean < /dev/null 2> tests/vm/mmap-clean.errors > tests/vm/mmap-clean.output
perl -I../.. ../../tests/vm/mmap-clean.ck tests/vm/mmap-clean tests/vm/mmap-clean.result
pintos -v -k -T 180 -m 10   —fs-disk=10 -p tests/vm/swap-anon:swap-anon —swap-disk=30 — -q   -f run swap-anon < /dev/null 2> tests/vm/swap-anon.errors > tests/vm/swap-anon.output
perl -I../.. ../../tests/vm/swap-anon.ck tests/vm/swap-anon tests/vm/swap-anon.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/pt-grow-stk-sc:pt-grow-stk-sc --swap-disk=4 -- -q   -f run pt-grow-stk-sc < /dev/null 2> tests/vm/pt-grow-stk-sc.errors > tests/vm/pt-grow-stk-sc.output
# perl -I../.. ../../tests/vm/pt-grow-stk-sc.ck tests/vm/pt-grow-stk-sc tests/vm/pt-grow-stk-sc.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/page-merge-stk:page-merge-stk -p tests/vm/child-qsort:child-qsort --swap-disk=10 -- -q   -f run page-merge-stk < /dev/null 2> tests/vm/page-merge-stk.errors > tests/vm/page-merge-stk.output
# perl -I../.. ../../tests/vm/page-merge-stk.ck tests/vm/page-merge-stk tests/vm/page-merge-stk.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/page-merge-mm:page-merge-mm -p tests/vm/child-qsort-mm:child-qsort-mm --swap-disk=10 -- -q   -f run page-merge-mm < /dev/null 2> tests/vm/page-merge-mm.errors > tests/vm/page-merge-mm.output
# perl -I../.. ../../tests/vm/page-merge-mm.ck tests/vm/page-merge-mm tests/vm/page-merge-mm.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/mmap-write:mmap-write --swap-disk=4 -- -q   -f run mmap-write < /dev/null 2> tests/vm/mmap-write.errors > tests/vm/mmap-write.output
# perl -I../.. ../../tests/vm/mmap-write.ck tests/vm/mmap-write tests/vm/mmap-write.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/mmap-exit:mmap-exit -p tests/vm/child-mm-wrt:child-mm-wrt --swap-disk=4 -- -q   -f run mmap-exit < /dev/null 2> tests/vm/mmap-exit.errors > tests/vm/mmap-exit.output
# perl -I../.. ../../tests/vm/mmap-exit.ck tests/vm/mmap-exit tests/vm/mmap-exit.result
# pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/vm/mmap-off:mmap-off -p ../../tests/vm/large.txt:large.txt --swap-disk=4 -- -q   -f run mmap-off < /dev/null 2> tests/vm/mmap-off.errors > tests/vm/mmap-off.output
# perl -I../.. ../../tests/vm/mmap-off.ck tests/vm/mmap-off tests/vm/mmap-off.result
# pintos -v -k -T 180 -m 8   --fs-disk=10 -p tests/vm/swap-file:swap-file -p ../../tests/vm/large.txt:large.txt --swap-disk=10 -- -q   -f run swap-file < /dev/null 2> tests/vm/swap-file.errors > tests/vm/swap-file.output
# perl -I../.. ../../tests/vm/swap-file.ck tests/vm/swap-file tests/vm/swap-file.result
# pintos -v -k -T 180 -m 10   --fs-disk=10 -p tests/vm/swap-anon:swap-anon --swap-disk=30 -- -q   -f run swap-anon < /dev/null 2> tests/vm/swap-anon.errors > tests/vm/swap-anon.output
# perl -I../.. ../../tests/vm/swap-anon.ck tests/vm/swap-anon tests/vm/swap-anon.result
# pintos -v -k -T 180 -m 10   --fs-disk=10 -p tests/vm/swap-iter:swap-iter -p ../../tests/vm/large.txt:large.txt --swap-disk=50 -- -q   -f run swap-iter < /dev/null 2> tests/vm/swap-iter.errors > tests/vm/swap-iter.output
# perl -I../.. ../../tests/vm/swap-iter.ck tests/vm/swap-iter tests/vm/swap-iter.result

cd ../..