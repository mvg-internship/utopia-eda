module AndAlways(input a, b, output o);
  always @(*)
    o <= a & b;
endmodule
