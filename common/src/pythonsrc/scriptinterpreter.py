def interpret(script):

    interpreterlines = script.split("\n")

    # checks for errors in string
    if not isinstance(script, str):
        raise TypeError("Wrong type of string in script!")

    for x in interpreterlines:
        statements = interpreterlines.split(" ")
        if statements[0] == "name":
            scriptname = statements[1]
        else:
            #To-Do
