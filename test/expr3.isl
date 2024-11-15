
entry {
    var iter : i64 = 3;
    loop (iter) {
        var hmm : string = "Iteration";
        print(hmm, iter);
        iter = iter as i64 - 1;
    }

    if (!0)
    {
        var then_s : string = "Then clause";
	    print("We are in: ", then_s);
    }
    else
    {
        var else_s : string = "Else clause";
        print("We are in: ", else_s);
    }

    var tmp : string = read("How old are you?");

    var asd : i64 = 3;
    {
        var tmp3 : i32 = 14 + 42;
        asd = asd + 15;
	    print("Result:", asd);
    }
    print(tmp, "years old?! Wow, that's old!");
}
