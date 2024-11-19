module main;

pure sum(a : i8, b : i8) -> i8 {
    print("The sum pure call was invoked!");
    return a + b;
}

pure sub(c : i8, d : i8) -> i8 {
    print("The sub pure call was invoked!");
    return c - d;
}

entry {  
    var iter : i32 = 3;
    var ch_t : i32 = 4;

    var verdict : i64 = (6 < 6) ? 0 : sum(3, 7);

    print("Sum result", verdict);

    loop (iter && ch_t) {
        var hmm : string = "Iteration";
        print(hmm, iter);
        iter = (iter as i64 - 1) as i32;
    }

    if (!false)
	    print("Second T");
    else
        print("Second F");

    if (!false)
    {
        var then_s : string = "Then clause";
	    print("We are in: ", then_s);
    }
    else
    {
        var else_s : string = "Else clause";
        print("We are in: ", else_s);
    }

    var tmp : i64 = read("Give me a number, please!") as i64;

    var asd : i64 = 3 as i32;
    {
        var tmp3 : i32 = (14 + 42) as i32;
        asd = asd + 15;
        print("Result:", asd);
    }
    print("You typed", tmp, "\nThat's an impressive number!");
}
