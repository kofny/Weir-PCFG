import re
import argparse
import os.path as path
import sys
from collections import defaultdict
from os import mkdir, walk, remove, rmdir


def read_pwd_set(filename):
    LDS = re.compile(r"([a-zA-Z]+|[0-9]+|[^\x00-\x20a-zA-Z0-9\x7f]+)")
    _structures = defaultdict(int)
    _digits = defaultdict(lambda: defaultdict(int))
    _symbol = defaultdict(lambda: defaultdict(int))
    with open(filename, "r") as fin:
        for line in fin:
            line = line.strip(" \r\n")
            groups = LDS.findall(line)  # type: [str]
            if len("".join(groups)) != len(line):
                continue
            s = ""
            for g in groups:  # type: str
                size = len(g)
                if g.isalpha():
                    s += ("L" * size)
                elif g.isdigit():
                    s += ("D" * size)
                    _digits[size][g] += 1
                else:
                    s += ("S" * size)
                    _symbol[size][g] += 1
            _structures[s] += 1
    return _structures, _digits, _symbol


def calc_prob(grammar: dict):
    sorted_grammar = sorted(grammar.items(), key=lambda x: x[1], reverse=True)
    total = sum(grammar.values())
    gram_prob_list = []
    for g, freq in sorted_grammar:
        gram_prob_list.append((g, float(freq) / total))
    return gram_prob_list


def remove_model(model_folder):
    for root, dirs, files in walk(model_folder):
        for name in files:
            remove(path.join(root, name))
        for name in dirs:
            remove_model(path.join(root, name))
            rmdir(path.join(root, name))
    pass


def create_dir(dir_to_be_created):
    try:
        mkdir(dir_to_be_created)
    except FileExistsError:
        pass
    except FileNotFoundError:
        sys.stderr.write(f"{dir_to_be_created} not found")
        sys.exit(1)


def write2disk(_structures: dict, _digits: dict, _symbol: dict, model_folder: str,
               grammar_folder="grammar", digits_folder="digit", symbol_folder="special"):
    remove_model(model_folder)
    create_dir(model_folder)
    create_dir(path.join(model_folder, grammar_folder))
    create_dir(path.join(model_folder, digits_folder))
    create_dir(path.join(model_folder, symbol_folder))
    f_struct = open(path.join(model_folder, grammar_folder, "structures.txt"), "w")
    for s in calc_prob(_structures):
        struct, prob = s
        f_struct.write(f"{struct}\x09{prob:.10f}\n")
    f_struct.flush()
    f_struct.close()
    for l, d in _digits.items():
        f_digits = open(path.join(model_folder, digits_folder, f"{l}.txt"), "w")
        for s in calc_prob(d):
            terminal, prob = s
            f_digits.write(f"{terminal}\x09{prob:.10f}\n")
        f_digits.flush()
        f_digits.close()
    for l, s in _symbol.items():
        f_symbol = open(path.join(model_folder, symbol_folder, f"{l}.txt"), "w")
        for terminal, prob in calc_prob(s):
            f_symbol.write(f"{terminal}\x09{prob:.10f}\n")
        f_symbol.flush()
        f_symbol.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Weir's PCFG Trainer")
    parser.add_argument("-p", "--pwd-set", required=True, help="password set, a password per line")
    parser.add_argument("-m", "--model", required=True, help="model will be save here")
    parser.add_argument("-g", "--grammar", required=False, default="grammar",
                        help="structures.txt will be saved in this directory")
    parser.add_argument("-d", "--digits", required=False, default="digits",
                        help="terminal of digits will be put in this directory")
    parser.add_argument("-s", "--symbol", required=False, default="special",
                        help="terminal of special will be put in this directory")
    args = parser.parse_args()
    structures, digits, symbol = read_pwd_set(args.pwd_set)
    write2disk(structures, digits, symbol, args.model, args.grammar, args.digits, args.symbol)
    print("Done!")
