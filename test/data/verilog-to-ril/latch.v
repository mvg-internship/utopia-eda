module Latch(clk, d, q);
  input clk, d;
  output reg q;
  always @(negedge(clk))
    q <= d;
endmodule
