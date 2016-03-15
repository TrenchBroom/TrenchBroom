from os import walk
from scriptinterpreter import interpret

if __name__ == "__main__":

    for root, dirs, files in walk("/plugins"):
        for file in files:
            if file.endswith(".tbs"):
                file.open()
                interpret(file.read())
                file.close()
