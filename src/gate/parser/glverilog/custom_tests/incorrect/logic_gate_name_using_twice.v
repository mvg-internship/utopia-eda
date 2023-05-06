module A(a,b,c);
input a;
output c;
wire w1;
not n1(w1,a);
not n1(c,w1);

endmodule