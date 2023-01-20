/*
    Test file for the final phase of HY-340: Languages & Compilers
    Computer science dpt, University of Crete, Greece

    Expected Output:
    user function 9
	user function 46
	4.000
	3.000
	error: line 25: call: cannot bind '3.000' to function!

*/

function executor (a) {
    f = (function(a){return(function(a){return(function(a){return(function(a){return a();});});});});
    print(f(a)(a)(a)(a), "\n");
}

t = [ { "tablefunc" : (function() { return (function(){return 3; }); }) }, { "tablefunc2" : (function(){return 4;}) } ];

executor(print);
executor(t.tablefunc);
executor(t.tablefunc2);
executor(t.tablefunc());
executor(t.tablefunc()());	//run time error


