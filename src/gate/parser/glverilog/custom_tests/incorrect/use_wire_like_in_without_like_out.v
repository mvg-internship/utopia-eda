module A(a,b,c);
input a;
output c,b;
wire w1;
not n1(b,a);
not n2(c,w1);

endmodule