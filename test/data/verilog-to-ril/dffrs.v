module DFFrs(clk, d, d1, rst, q);
  input clk, d, d1, rst;
  output reg q;

  always @(posedge(clk))
    if (rst)
      q <= d1;
    else
      q <= d;
endmodule
