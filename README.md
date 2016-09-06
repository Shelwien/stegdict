# stegdict
Steganography based on permutations of sorted dictionary

The main idea is that dictionary file compression is a big problem for any fast codecs.
So here's a workaround - if we cannot take care of some correlations in the data,
let's randomize them instead via steganography, and get better overall compression as a result.
Its the same idea that I proposed for jpeg/mp3 in http://encode.ru/threads/1405-Stegan...-lossy-formats

To compile: 
> g++ -O3 -s stegdict.cpp -ILib -o stegdict

To use: 
> Encode: stegdict c input_dict output_dict payload  
> Decode: stegdict d input_dict sorted_dict payload.unp
For fixed-size records:  
> Encode: stegdict c16 input_dict output_dict payload  
> Decode: stegdict d16 input_dict sorted_dict payload.unp
For config file  
> Encode: stegdict c @config1 payload  
> Decode: stegdict d @config2 payload.unp  
config1:  
\#16:input_dict output_dict  
config2:  
\#16:input_dict sorted_dict  
 
Contacts: shelwien.san@gmail.com
