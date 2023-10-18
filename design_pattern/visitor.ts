interface PartryVisitor {
    visitBeignet(beignet: Beignet): void;
    visitCruller(cruller: Cruller): void;
}

// OOBSTRUCTURE

abstract class Pastry {
    abstract accept(visitor: PartryVisitor): void;
}

class Cook implements Pastry{
    private list: Pastry[] = [];

    public accept(visitor: PartryVisitor): void {
        this.list.map(item => item.accept(visitor));
    }

    public add(p: Pastry) {
        this.list.push(p);
    }

    public remove(p: Pastry) {
        this.list.pop();
    }
}


class Beignet extends Pastry {
    accept(visitor: PartryVisitor): void {
        visitor.visitBeignet(this);
    }

    public callName() {
        console.log("Beignet");
    }
}

class Cruller extends Pastry {
    accept(visitor: PartryVisitor): void {
        visitor.visitCruller(this);
    }

    public callName1() {
        console.log("Cruller");
    }
}


class ChildVisitor implements PartryVisitor {
    visitBeignet(beignet: Beignet): void {
        beignet.callName();
    }

    visitCruller(cruller: Cruller): void {
        cruller.callName1();
    }
}


const cooker = new Cook();

cooker.add(new Beignet());
cooker.add(new Cruller());

cooker.accept(new ChildVisitor());
