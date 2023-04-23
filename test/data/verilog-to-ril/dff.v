module Dff(clk, d, o);
  input clk, d;
  output reg o;

  always @(posedge(clk))
    o <= d;
endmodule
