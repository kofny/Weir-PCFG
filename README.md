# Weir-PCFG

Weir's PCFG, rewrite train in python3

## Specification

This project is based on Weir's Probabilistic Cracker.
I rewrite the trainer in python3.6.8 and add add some cli into it.

The guesser reused Weir's codes too. Since it's not necessary to 
completely rewrite the guesser, I add some cli and improve the 
performance of Weir's code.

By the way, there may be bugs which I failed to find out. 
So keep in mind that if there are any question, 
re-run the Original Weir's PCFG here.
## Usage
### init.sh

This script will firstly clean the trainer and guesser, as well as 
some middleware, and then re-generate them.

```shell script
./init.sh
```

### trainer

You need to specify the python interpreter, such as `source venv/bin/python`

```shell script
$ python train.py -h
usage: Weir's PCFG Trainer [-h] -p PWD_SET -m MODEL [-g GRAMMAR] [-d DIGITS]
                           [-s SYMBOL]
```

### guesser

It's a little complex.
The **required** arguments are:

1. --model
2. --guesses-file
3. --guess-number
4. -dname0

```shell script
PCFG MANAGER - A password guess generator based on probablistic context free grammars
Created by Matt Weir, weir@cs.fsu.edu
Special thanks to Florida State University and the National Institute of Justice for supporting this work
----------------------------------------------------------------------------------------------------------
Usage Info:
./pcfg_manager <options>
        Options:
        --model <model directory>       <REQUIRED>: The trained model path
        --guesses-file <guesses file>   <REQUIRED>: guesses generated will be put here
        --guess-number <guess number>   <REQUIRED>: the guess number
        --grammar <grammar folder>      <OPTIONAL>: grammar directory name under model path
        --digits <digits folder>        <OPTIONAL>: digits directory name under model path
        --symbol <symbol folder>        <OPTIONAL>: symbol directory name under model path
        -dname[0-9] <dictionary name>   <REQUIRED>: The input dictionary name
                Example: -dname0 common_words.txt
        -dprob[0-9] <dictionary probability>    <OPTIONAL>: The input dictionary's probability, if not specified set to 1.0
                Example: -dprob0 0.75
        -removeUpper            <OPTIONAL>: don't include dictionary words that contain uppercase letters
        -removeSpecial          <OPTIONAL>: don't include dictionary words that contain special characters
        -removeDigits           <OPTIONAL>: don't include dictionary words that contain digits
```

