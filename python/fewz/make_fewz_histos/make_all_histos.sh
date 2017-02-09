#!/bin/bash

for f in ./fewz_predictions/*.txt; do
  echo ""
  echo "--------------------------------------------"
  echo "Making histograms from $f ..."
  echo ""
  python parseFEWZtoTH1Fs.py --infile $f
  echo "Done."
  echo ""
done
