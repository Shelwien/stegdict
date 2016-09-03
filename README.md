# stegdict
Steganography based on permutations of sorted dictionary

The main idea is that dictionary file compression is a big problem for any fast codecs.
So here's a workaround - if we cannot take care of some correlations in the data,
let's randomize them instead via steganography, and get better overall compression as a result.
Its the same idea that I proposed for jpeg/mp3 in http://encode.ru/threads/1405-Stegan...-lossy-formats

To compile: 
> g++ -m64 -O3 -s stegdict.cpp -ILib -o stegdict

To use: 
> Encode: stegdict c input_dict output_dict payload  
> Decode: stegdict d input_dict sorted_dict payload.unp
 
Contacts: shelwien.san@gmail.com
