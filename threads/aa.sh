make clean
make
cd build
# pintos -v -k -T 60 -m 20   -- -q   run priority-donate-chain < /dev/null 2> tests/threads/priority-donate-chain.errors > tests/threads/priority-donate-chain.output
# perl -I../.. ../../tests/threads/priority-donate-chain.ck tests/threads/priority-donate-chain tests/threads/priority-donate-chain.result
# pintos -v -k -T 60 -m 20   -- -q   run priority-donate-multiple < /dev/null 2> tests/threads/priority-donate-multiple.errors > tests/threads/priority-donate-multiple.output
# perl -I../.. ../../tests/threads/priority-donate-multiple.ck tests/threads/priority-donate-multiple tests/threads/priority-donate-multiple.result
# pintos -v -k -T 60 -m 20   -- -q   run priority-donate-multiple2 < /dev/null 2> tests/threads/priority-donate-multiple2.errors > tests/threads/priority-donate-multiple2.output
# perl -I../.. ../../tests/threads/priority-donate-multiple2.ck tests/threads/priority-donate-multiple2 tests/threads/priority-donate-multiple2.result
# pintos -v -k -T 60 -m 20   -- -q   run priority-donate-nest < /dev/null 2> tests/threads/priority-donate-nest.errors > tests/threads/priority-donate-nest.output
# perl -I../.. ../../tests/threads/priority-donate-nest.ck tests/threads/priority-donate-nest tests/threads/priority-donate-nest.result
pintos -v -k -T 60 -m 20   — -q   run priority-donate-one < /dev/null 2> tests/threads/priority-donate-one.errors > tests/threads/priority-donate-one.output
perl -I../.. ../../tests/threads/priority-donate-one.ck tests/threads/priority-donate-one tests/threads/priority-donate-one.result
# pintos -v -k -T 60 -m 20   — -q   run priority-donate-sema < /dev/null 2> tests/threads/priority-donate-sema.errors > tests/threads/priority-donate-sema.output
# perl -I../.. ../../tests/threads/priority-donate-sema.ck tests/threads/priority-donate-sema tests/threads/priority-donate-sema.result
# pintos -v -k -T 60 -m 20   — -q   run priority-donate-lower < /dev/null 2> tests/threads/priority-donate-lower.errors > tests/threads/priority-donate-lower.output
# perl -I../.. ../../tests/threads/priority-donate-lower.ck tests/threads/priority-donate-lower tests/threads/priority-donate-lower.result