module halfSubtractor(x0, x1, x2, x3, x4);
  input x0, x1;
  output x2, x4;
  wire x2,x3,x4;

  xor g1 (x2, x0, x1);
  not g2 (x3, x0);
  nand g3 (x4, x3, x1);
endmodule
