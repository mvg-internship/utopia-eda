module A(a,b,c);
input a,b;
output c,d;
not n1(c,a);
endmodule

module B(a,b,c);
input a,b;
output c,b;
not n2(c,a);
endmodule
