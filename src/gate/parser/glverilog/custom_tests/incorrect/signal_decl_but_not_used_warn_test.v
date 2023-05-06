module A(a,b,c,d);
input a,b;
output c,d;
dff d1(a,c,b);
endmodule

module B(a,b,c,d);
input a,b,d;
output c;
dff d1(a,c,b);
endmodule