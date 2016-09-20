# stegdict
Steganography based on permutations of sorted dictionary  
http://encode.ru/threads/2597-Steganography-based-on-dictionary-permutations

The main idea is that dictionary file compression is a big problem for any fast codecs.
So here's a workaround - if we cannot take care of some correlations in the data,
let's randomize them instead via steganography, and get better overall compression as a result.
Its the same idea that I proposed for jpeg/mp3 in  
http://encode.ru/threads/1405-Steganography-for-universal-recompression-of-lossy-formats

To compile: 
> g++ -O3 -s stegdict.cpp -ILib -o stegdict

To use: 
> Encode: stegdict c input_dict output_dict payload  
> Decode: stegdict d input_dict sorted_dict payload.unp

*Sample files can be taken from release archives - https://github.com/Shelwien/stegdict/releases  
(There're only windows executables, but sample files are universal).
Also there're samples in http://nishi.dreamhosters.com/u/fplog_demo_0.rar*

Example:  
> stegdict c LICENSE license_mut README.md  
> stegdict d license_mut license_sort payload  
*Payload size should be 535 bytes*
  
For fixed-size records:  
> Encode: stegdict c16 input.bin output.bin payload  
> Decode: stegdict d16 input.bin sorted.bin payload.unp

For config file  
> Encode: stegdict c @config1 payload  
> Decode: stegdict d @config2 payload.unp  

config1:  
> \#16:input.bin output.bin  
>      input2.bin output2.bin  

config2:  
> \#16:input.bin sorted.bin  
>      input2.bin sorted2.bin  
 
Contacts: shelwien@gmail.com
