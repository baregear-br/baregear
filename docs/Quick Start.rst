Quick Start
===========

Writing Simple Program
----------------------
1. Hello World Program:

.. code-block:: baregear
    print "Hello World!"

- Replace 'Hello World' With text if you want And Save The File On Disk.

.. code-block:: bash
    $ baregear helloWorld.br -o helloWorld
    $ ./helloWorld

- Replace helloWorld.br with actual file name.

prints Hello World Or Any Text:

.. code-block:: bash
    Hello World!

1. Simple Calculator:

.. code-block:: baregear
    x = input "Enter Your First Number: "
    op = input "Enter Your Operator To Calculate: "
    y = input "Enter Your Second Number: "

    switch op:
        case '+':
            print 'Result: ' + (x + y)

        case '-':
            print 'Result: ' + (x - y)

        case '*':
            print 'Result: ' + (x * y)

        case '/':
            print 'Result: ' + (x / y)

        default:
            print 'Invalid Operator: '
        
But You Won't Implement The if, else for fixing the
problem of Devide By Zero Or Any Compiler Handle It,
And Compiler Also Implement User If Typed Wrong Operator
To Doesn't Accept And Show Error 'Invalid Operator Please
Enter Operator From +, -, *, /'
When User Typed Any Operator To Automatically Take
Without Pressing Enter Key And If Bugs Found By Simulating
The Program To Fix By Compiler If Solution Available
Otherwise Throw Error.

Then Follow Same Compilation Step

Ouput Looks Like:
.. code-block:: bash
    Enter First Number: 49
    Enter Operator: *
    Enter Second Number: 273
    Result: 13377

If User Do Mistake:
.. code-block:: bash
    Enter First Number: 49
    Enter Operator: -:-
    Invalid Operator Please Enter Operator From +, -, *, /
    Enter Operator: /
    Enter Second Number: 0
    Cannot Devide By Zero
    Enter Second Number: 2
    Result: 24.5

3. Guess The Number Game

.. code-block:: baregear
    attempts = 0
    number = random 1 to 100
    print "Guess The Number From 1 to 100 But You Can Provide Number Upto 5 Times"
    
    while attempts < 5:
        userInput = input "Enter Your Number: ".
        if userInput > number:
            print "Your Number Greater Then My Number."
        else if userInput < number:
            print "Your Number Shorter Then My Number."
        else:
            print "Matched."
            exit 0
        attempts += 1
    
    print "You Cannot Guess Valid Number."

- Alternative You Can Use switch
- Alternative else if keyword is 'elif'

Comments
--------

Single Line Comment:

.. code-block:: baregear
    # This Is Single Line Comment

.. code-block:: baregear
    ---
    This Is Multiline Comment
    baregear ignores the comment
    ---

Data Types
----------
baregear allows the dynamic typing
but you can limit to use the datatype
of variable.

Simple Calculator Has Bug If User Typed The Text
When User Enter Text Instead Of Number:
.. code-block:: bash
    Enter First Number: hi
    Enter Operator: -
    Operator Not Supported
    Enter Operator: *
    Operator Not Supported
    Enter Operator: /
    Operator Not Supported
    Enter Operator: +
    Enter Second Number: 48
    Result hi48

You Can Use numb to block the inputting text:
.. code-block:: baregear
    numb x = input "Enter Your First Number: "
    op = input "Enter Your Operator To Calculate: "
    numb y = input "Enter Your Second Number: "

    # Next Code Remains Same

After Applying The Code:
.. code-block:: bash
    Enter First Number: h
    Only Numbers Allowed.
    Enter First Number:

List Of Datatypes:
- variant datatype variable supports all datatypes
- `text`: Allow To Store And Load String
- `numb`: Supports `int`, `float`, `double`, `short`, `long` Value Store
And Loading And Auto Dynamic Allocation
- `short`: Used to store the the value in limit but not support dynamic Allocation
- `double`: Used to store accruate decimal Numbers
- `float`: Used to store the decimal Numbers
- `int`: Used To Store Only Non decimal number

And Guess The Number Game Make To Easier To Gess:
.. code-block:: baregear
    attempts = 0
    int number = random 1 to 100
    print "Guess The Number From 1 to 100 But You Can Provide Number Upto 5 Times"
    
    while attempts < 5:
        int userInput = input "Enter Your Number: ".
        if userInput > number:
            print "Your Number Greater Then My Number."
        else if userInput < number:
            print "Your Number Shorter Then My Number."
        else:
            print "Matched."
            exit 0
        attempts += 1
    
    print "You Cannot Guess Valid Number."

Creating Custom Variant Variable:
.. code-block:: baregear
    text, int alphabetNumber

And When You Store Number With Decimal Or Text on 'alphabetNumber'
To Asign as text Otherwise You Store Only Number To Asign as int