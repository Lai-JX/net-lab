# CMake generated Testfile for 
# Source directory: D:/hgs-3-2/cpmpute-net/lab/net-lab-master
# Build directory: D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(eth_in "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/eth_in.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/eth_in")
set_tests_properties(eth_in PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;117;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
add_test(eth_out "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/eth_out.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/eth_out")
set_tests_properties(eth_out PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;122;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
add_test(arp_test "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/arp_test.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/arp_test")
set_tests_properties(arp_test PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;127;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
add_test(ip_test "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/ip_test.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/ip_test")
set_tests_properties(ip_test PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;132;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
add_test(ip_frag_test "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/ip_frag_test.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/ip_frag_test")
set_tests_properties(ip_frag_test PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;137;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
add_test(icmp_test "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/build/icmp_test.exe" "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/testing/data/icmp_test")
set_tests_properties(icmp_test PROPERTIES  _BACKTRACE_TRIPLES "D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;142;add_test;D:/hgs-3-2/cpmpute-net/lab/net-lab-master/CMakeLists.txt;0;")
