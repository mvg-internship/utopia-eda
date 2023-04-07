
module BUF(A, Y);
input A;
output Y = A;
endmodule

module NOT(A, Y);
input A;
output Y = ~A;
endmodule

module NAND(A, B, Y);
input A, B;
output Y = ~(A & B);
endmodule

module NOR(A, B, Y);
input A, B;
output Y = ~(A | B);
endmodule

module DFF(C, D, Q);
input C, D;
output Q;
reg T;

always @(posedge C)
	Q <= T;
always @(negedge C)
	T <= D;
endmodule


