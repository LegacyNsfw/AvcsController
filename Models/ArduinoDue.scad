// Dimensions
totalWidth = 190.5; // 7.5"
totalLength = 190.5;
inch = 25.4;
halfInch = inch / 2;
quarterInch = inch / 4; // .25"
floorDepth = quarterInch;   // Real = .25 inches
edgeWidth = quarterInch;
bossHeight = quarterInch;
cavityDepth = 0.1 * inch;       // Real = 2 inches
boltRadius = 1.4;
dividerHeight = 56;
dividerDepth = inch;

$fn = 10;

// Boss
module boss(x, y)
{
    translate ([x, y, floorDepth + (bossHeight / 2) - 1])
    {
        difference()
        {
            cylinder(h = bossHeight, r = quarterInch, center = true, $fn = 20);
            cylinder(h = bossHeight + 1, r = boltRadius, center = true, $fn = 10);
        }
    }
}

difference()
{
    union()
    {
        cube([4.25 * inch, 2.75 * inch, floorDepth], false);

        // Arduino
        translate([0, 10, 0])
        {
            boss (14.25, 2.5); //  lower left
            boss (16,             2.5 + 5.1 + 27.9 + 15.2); // top left
            boss (14    + 52.5,        2.5 + 5.1); // lower Uno hole
            boss (14    + 52.5,        2.5 + 5.1 + 27.9); // upper Uno hole
            boss (14.05 + 52.5 + 24.1, 2.5 + 5.1 + 27.9 + 15.2); // upper right
            boss (12.00 + 85.3,        2.5); // lower right
        }
    }
    
    translate ([.25 * inch, .8 * inch, -10])
    {
        cube ([2 * inch, 1.2 * inch, inch], false);
    }
        
    translate ([inch, 0.25 * inch, -10])
    {
        cube ([1.25 * inch, 2.25 * inch, inch], false);
    }
    
    translate ([3 * inch, .85 * inch, -10])
    {
        cube ([1 * inch, 1.2 * inch, inch], false);
    }
}
