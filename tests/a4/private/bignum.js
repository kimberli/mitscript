List = function() {
  return {
    head: null,
    len: function() {
      if (this.head == null) {
        return 0;
      } else {
        return this.head.len;
      }
    },
    add: function(v) {
      this.head = {
        len: (this.len()+1),
        car: v,
        cdr: this.head,
      }
    },
    print: function() {
      t = this.head;
      s = "";
      while (t != null) {
        s = s + (t.car) + ", ";
        t = t.cdr;
      }
      console.log(s);
    },
    forall: function(f) {
      t = this.head;
      while (t != null) {
        f(t.car);
        t = t.cdr;
      }
    },
    clone: function() { 
      rv = List();
      clone = function(n) {
        if (n == null){
          return null;
        } else {
          return {len:n.len, car:n.car, cdr:clone(n.cdr)};
        }
      };
      rv.head = clone(this.head);
      return rv;
    }
  };
};


modN = function(lst, N) {
  rv = List();
  lst.forall(function(n) {
    while (n >= N) {
      n = n - N;
    }
    rv.add(n);
  });
  return rv;
};

pair = function(l1, l2) {
  while (l1.len() > l2.len()) {
    l2.add(0);
  } 
  while (l2.len() > l1.len()) {
    l1.add(0);
  }
  state = {node:l1.head};
  rv = List();
  l2.forall(function(v) { 
    rv.add({v1:v, v2:state.node.car});
    state.node = state.node.cdr;
  });
  return rv;
};

plus = function(a, b, N) {
  temp = pair(a, b);
  state = {carry:0};
  rv = List();  
  temp.forall(function(pair) {
    v = pair.v1 + pair.v2 + state.carry;
    state.carry = 0;
    while (v >= N) {
      v = v - N;
      state.carry = state.carry + 1;
    } 
    rv.add(v);  
  }); 
  if (state.carry != 0) {
    rv.add(state.carry);
  }
  return rv;
};


mult = function(a, smallb, N) {
  i=0;
  v = List();
  v.add(0);
  while (i < smallb) {
    v = plus(v, a, N);   
    i = i + 1;
  }
  return v;
};


N = 99;
l = List();
i = 0;
BLAH = [5, 6, 12, 4, 2, 6, 7, 8, 9, 88, 8, 3, 4, 1, 5, 9, 0, 0, 0, 1, 1, 3, 5, 9, 2, 2, 1, 9, 7, 9, 5, 3, 2, 1, 8, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 38, 2, 1, 3, 9, 2, 4, 5, 3];
while (i < N) {
  l.add(BLAH[i]);
  i = i + 1;
}
BASE = 10;
console.log("After read");
l = modN(l, BASE);
console.log("After Mod");
l.print();
l2 = plus(l, l, BASE);
console.log("After plus");
l4 = plus(l2, l2, BASE);
l.print();
l2.print();
l4.print();

i=0; 
while(i < 110){
  console.log(" x " + i);
  l5 = mult(l4, i, BASE);
  l5.print();
  i = i + 1;
}


