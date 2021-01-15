# Locally Likely Arrangement Hashing (LLAH) --- Source codes written by Tomohiro Nakai
This repository contains the source code of the real-time camera-captured document image retrieval method named Locally Likely Arrangement Hashing (LLAH) written by Tomohiro Nakai.
 
## Old stable LLAH source code
Directory: ./old_stable
Visual Studio Project: ./vs/llahdoc080620/

This directory contains an old version of LLAH program, which is fast and stable.

Usage: 
 - Database construction: llahdoc -c [document image directory] [document image suffix] [option]
 - Retrieval: llahdoc [option] [query image]
 - Real-time retrieval: llahdoc -S [option] (server.ini is needed to be setup)
 
See Function *AnalyzeArgAndSetExtern2* in *init.c* for the options.

References:
- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura. Hashing with Local Combinations of Feature Points and Its Application to Camera-Based Document Image Retrieval ---Retrieval in 0.14 Second from 10,000 Pages---.
 Proc. First International Workshop on Camera-Based Document Analysis and Recognition (CBDAR2005), pp.87-94 (2005-8).


- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Camera-Based Document Image Retrieval as Voting for Partial Signatures of Projective Invariants.
 Proc. 8th International Conference on Document Analysis and Recognition (ICDAR2005), pp.379-383 (2005-9).
 
- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Use of Affine Invariants in Locally Likely Arrangement Hashing for Camera-Based Document Image Retrieval.
 Lecture Notes in Computer Science (7th International Workshop DAS2006), 3872, pp.541-552, Nelson, New Zealand (2006-2).

- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Camera Based Document Image Retrieval with More Time and Memory Efficient LLAH.
 Proc. Second International Workshop on Camera-Based Document Analysis and Recognition (CBDAR2007), pp.21-28 (2007-9).

- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Real-Time Document Image Retrieval with More Time and Memory Efficient LLAH.
 Proc. Second International Workshop on Camera-Based Document Analysis and Recognition (CBDAR2007), pp.168-169 (2007-9).

- Masakazu Iwamura, Tomohiro Nakai, and Koichi Kise.
 Improvement of Retrieval Speed and Required Amount of Memory for Geometric Hashing by Combining Local Invariants.
 Proc. 18th British Machine Vision Conference (BMVC2007), 2, pp.1010-1019 (2007-9).

- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Real-Time Retrieval for Images of Documents in Various Languages.
 Handout of 8th IAPR International Workshop on Document Analysis Systems (DAS2008), p.37 (2008-9).

- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 Real-Time Retrieval for Images of Documents in Various Languages using a Web Camera.
 Proceedings of the 10th International Conference on Document Analysis and Recognition (ICDAR2009), pp.146-150, Barcelona, Spain (2009-7).



## Annotation extraction
Directory: ./anno_extract
Visual Studio Project: ./vs/annex

Usage: annex [option] [original image] [annotated image]

See function *InitAnnex* in *annex.c* for the options.

Reference:
- Tomohiro Nakai, Koichi Kise, Masakazu Iwamura.
 A Method of Annotation Extraction from Paper Documents Using Alignment Based on Local Arrangements of Feature Points.
 Proc. 9th International Conference on Document Analysis and Recognition (ICDAR2007), 1, pp.23-27 (2007-9).


## Mosaicing
Directory: ./llahmos
Visual Studio Project: ./vs/llahmos

Usage: llahmos [option] [image1] [image2]

See function *AnalyzeArgAndSetExtern2* in *annex.c* for the options.

Reference:
- Tomohiro Nakai, Koichi Kise, and Masakazu Iwamura.
 Camera-based document image mosaicing using LLAH.
 Proc. Document Recognition and Retrieval XVI (Proc. SPIE Volume 7247), 7247-25 (2009-1).



## Demo server
Directory: ./demo_server
Visual Studio Project: ./vs/llahdoc

This directory contains the server program for the real-time demonstration.

Usage: 
 - Database construction: llahdoc -c [document image directory] [document image suffix] [option]
 - Retrieval: llahdoc [option] [query image]
 - Real-time retrieval: llahdoc -S [option] (*server.ini* is needed to be setup)
 
See function *AnalyzeArgAndSetExtern2* in *init.c* for the options.


## Demo client
Visual Studio Project: ./vs/dcamc

Usage:
 1. Launch the server program
 2. Plug in a web camera
 3. Run the client program

*client.ini* is needed to be setup (In particularly, the name of client should be correctly written).

