module xorGate(cOut, x0, x1);
  input x0, x1;
  output cOut;
  wire cOut;

  xor g4 (cOut, x0, x1);
endmodule
