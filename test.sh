./simulation.o inputs/configuration_file_1.txt output1.txt
./simulation.o inputs/configuration_file_2.txt output2.txt
./simulation.o inputs/configuration_file_4.txt output4.txt
./simulation.o inputs/configuration_file_6.txt output6.txt
./simulation.o inputs/configuration_file_7.txt output7.txt
./simulation.o inputs/configuration_file_8.txt output8.txt
diff outputs/out_configuration_file_1.txt output1.txt
diff outputs/out_configuration_file_2.txt output2.txt
diff outputs/out_configuration_file_4.txt output4.txt
diff outputs/out_configuration_file_6.txt output6.txt
diff outputs/out_configuration_file_7.txt output7.txt
diff outputs/out_configuration_file_8.txt output8.txt

