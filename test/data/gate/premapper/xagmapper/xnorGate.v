module xnorGate(cOut, x0, x1);
  input x0, x1;
  output cOut;
  wire cOut;

  xnor g4 (cOut, x0, x1);
endmodule
