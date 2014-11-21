Catalano-Fiore
==============

Implementation of the [Catalano-Fiore] [CF13] scheme.

Dependencies
--------------
* [gmp] [gmp] The number library
* [libgcrypt] [gcrypt] For AES
* [pinocchio] [pinocchio] (Optional) For C to circuit compiler
* [NTL] [NTL] For FFT multiplication

Usage
-------------
To use the protocol there is the executable `src/cf`. The following usages are available:

* **Keygen**: `./src/cf keygen [-n N_BITS -o KEY_FILE]`. This generates the key pair of `N_BITS` bits (defaults to 128). The evaluation key is stored in `KEY_FILE.ek` and the secret key in `KEY_FILE.sk`.
* **Message Authentication**: `./src/cf auth -s KEY_FILE.sk -e KEY_FILE.ek -m MESSAGE -l LABEL [-o OUTFILE]`. Authenticates the message `MESSAGE` wrt label `LABEL`. It is possible to authenticate a single message, and in that case `MESSAGE` and `LABEL` must be hexadecimal strings. To authenticate multiple messages at once, `MESSAGE` and `LABEL` must be files, where each line represent a message/label. The line count of both files must be the same.
* **Homomorphic Evaluation**: `./src/cf eval -e KEY_FILE.ek -a AC_FILE -t TAGS [-o OUTFILE]`. Performs the homomorphic evaluation over the previously authenticated messages represented by the `TAGS` file. The line count of this file must be exactly the same as the number of inputs of the Arithmetic Circuit represented in `AC_FILE`. See the Arithmetic Circuits section for more information about them. The resulting tag should be written to `OUTFILE`.
* **Verification**: `./src/cf vrfy -s KEY_FILE.sk -e KEY_FILE.ek -m MESSAGE -a AC_FILE -l LABELS -t EVAL_TAG`. Verifies if the `EVAL_TAG` obtained by a previous homomorphic evaluation is valid. `MESSAGE` can either be the evaluated message (in hexadecimal format) or a file containg multiple messages (the number of messages in that file must be the same number of labels in the `LABELS` file and the same number of inputs of the Arithmetic Circuit `AC_FILE`) that are later evaluated by `./src/cf`. In the end, `TRUE` or `FALSE` is printed on the screen.
* **Message Evaluation**: `./src/cf msgeval [-e KEY_FILE.ek] -m MESSAGES -a AC_FILE [-o OUTFILE]`. Evaluates the messages in `MESSAGES` file over the Arithmetic Circuit `AC_FILE`. If the evaluation key is supplied, then operations are done modulo `p`.


In the `test/` there are some tests to the protocol. To see which ones are
available run `test/test --help`.

Pinocchio
----------------------------------
* Download the latest release from [codeplex] [pinocchio]
* It relies on **python2**
* `python2 ccompiler/src/aritheval.py AC_FILE INPUT_FILE OUTPUT_FILE` is the arithmetic circuit evaluator.

Helper scripts
-----------------
There are some helper scripts to generate random messages and labels.

* Generation of labels: `src/generator.py label N_LABELS N_BITS --out OUTFILE`. Generates `N_LABELS` labels with exactly `N_BITS` each and stores them in `OUTFILE`.
* Generation of messages: `src/generator.py message N_MSGS MODULO --out OUTFILE`. Generates `N_MSGS` messages modulo `MODULO` and stores them in `OUTFILE`.

These generated labels and messages can then be used as inputs to the CF protocol.

LaTeX: Compilation instructions
----------------------------------
1. Install `texlive2013` which includes `xelatex` and `latexmk`
1. By default, `xelatex` (through `latexmk`) is the default compiler. To change
   to `pdflatex`, just edit the `$pdflatex` variable of `tex/latexmkrc`.
1. To compile, `make pdf`.
1. Walk through `tex/Makefile` and `tex/latexmkrc` if you wish to make further
   changes.


[CF13]:http://link.springer.com/chapter/10.1007%2F978-3-642-38348-9_21
[gcrypt]:https://www.gnu.org/software/libgcrypt/
[pinocchio]:https://vc.codeplex.com/
[gmp]:https://gmplib.org/
[sage]:http://www.sagemath.org/
[NTL]:http://www.shoup.net/ntl/
