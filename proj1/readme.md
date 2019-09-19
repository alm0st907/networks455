run the "run" binary as described in the assignment within mininet

The program may behave slightly weird in vm's (it did on my mac) but ran fine on a dual boot system

./run Send \<interfaceName> \<DestHWAddy> \<message>

./run Recv \<interfaceName>

if the program needs to be recompiled for any reason, run the mk.sh which will remove the old binary and recompile under the same name