make
./distvec examples/$1/topology.txt examples/$1/messages.txt examples/$1/changes.txt
./linkstate examples/$1/topology.txt examples/$1/messages.txt examples/$1/changes.txt
diff -s output_dv.txt examples/$1/output_dv.txt
diff -s output_ls.txt examples/$1/output_ls.txt

