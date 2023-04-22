module nandGate(cOut, x0, x1);
  input x0, x1;
  output cOut;
  wire cOut;

  nand g1 (cOut, x0, x1);
endmodule
