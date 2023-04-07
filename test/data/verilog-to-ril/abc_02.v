module BUF(A, C, Y);
input A,C;
output Y = A&C;
endmodule

module NOT(A, Y);
input A;
output Y = ~A;
endmodule


module FFFF(C, C1, D, D1, D2,D3, Q);
input C, C1, D, D1, D2,D3;
output reg Q;
reg T;
always @(posedge C)
	Q <= T;
always @(posedge C1)
	T <= D;
endmodule
