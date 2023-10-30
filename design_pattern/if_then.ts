// SmallTalk implemnet

class True {
    ifThen(thenBranch: Function) {
        return thenBranch();
    }

    ifThenElse(thenBranch: Function, elseBranch: Function) {
        return thenBranch();
    }
}

class False {
    ifThen(thenBranch: Function) {
        return null;
    }

    ifThenElse(thenBranch: Function, elseBranch: Function) {
        return elseBranch();
    }
}

const t = new True();
const f = new False();

function test(condition) {
    function ifThenFn() {
        console.log("if then -> then");
    }

    condition.ifThen(ifThenFn);

    function ifThenElseThenFn() {
        console.log("if then else -> then");
    }

    function ifThenElseElseFn() {
        console.log("if then else -> else");
    }

    condition.ifThenElse(ifThenElseThenFn, ifThenElseElseFn);
}

test(t);
test(f);
