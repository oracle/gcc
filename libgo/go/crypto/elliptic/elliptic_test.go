// Copyright 2010 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package elliptic

import (
	"testing"
)

func TestInfinity(t *testing.T) {
	tests := []struct {
		name  string
		curve Curve
	}{
		{"p256", P256()},
	}

	for _, test := range tests {
		curve := test.curve
		x, y := curve.ScalarBaseMult(nil)
		if x.Sign() != 0 || y.Sign() != 0 {
			t.Errorf("%s: x^0 != ∞", test.name)
		}
		x.SetInt64(0)
		y.SetInt64(0)

		x2, y2 := curve.Double(x, y)
		if x2.Sign() != 0 || y2.Sign() != 0 {
			t.Errorf("%s: 2∞ != ∞", test.name)
		}

		baseX := curve.Params().Gx
		baseY := curve.Params().Gy

		x3, y3 := curve.Add(baseX, baseY, x, y)
		if x3.Cmp(baseX) != 0 || y3.Cmp(baseY) != 0 {
			t.Errorf("%s: x+∞ != x", test.name)
		}

		x4, y4 := curve.Add(x, y, baseX, baseY)
		if x4.Cmp(baseX) != 0 || y4.Cmp(baseY) != 0 {
			t.Errorf("%s: ∞+x != x", test.name)
		}
	}
}
