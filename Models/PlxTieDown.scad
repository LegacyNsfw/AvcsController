postSpacing = 78-14; // coordinates copied from Case.scad


inch = 25.4;
halfInch = inch / 2;
quarterInch = inch / 4;
thickness = quarterInch;
boltRadius = 1.8; // large enough for the bolt to drop through
boltHeadRadius = 3;
bossHeight = thickness;
halfSpan = 50;
anchorDepth = 9;

$fn = 50;

module boss(x, y, solid)
{
    translate ([x, y, -anchorDepth / 2])
    {
        if (solid > 0)
        {
            cylinder(h = bossHeight, r = quarterInch, center = true, $fn = 20);
            cylinder(h = anchorDepth + bossHeight, r1 = quarterInch, r2 = halfInch, center = true, $fn = 20);
        }
        else
        {
            cylinder(h = bossHeight + 30, r = boltRadius, center = true, $fn = 10);
            
            translate ([0, 0, bossHeight])
                cylinder(h = anchorDepth + bossHeight + 0.01, r1 = boltRadius, r2 = halfInch - boltHeadRadius, center = true, $fn = 20);
        }
    }
}

module disc(x, y)
{
    translate ([x, y, 0])
    {
        cylinder(h = bossHeight, r = quarterInch, center = true, $fn = 20);
    }
}

module bosses(solid)
{
    boss(0, 0, solid);
    boss(0, postSpacing, solid);
}

module frame()
{
    hull()
    {
        disc(0, 0);
        disc(-halfSpan, postSpacing / 2);
    }
    
    hull()
    {
        disc(-halfSpan, postSpacing / 2);
        disc(0, postSpacing);
    }
    
    hull()
    {
        disc(0, postSpacing);
        disc(halfSpan, postSpacing / 2);
    }
    
    hull()
    {
        disc(halfSpan, postSpacing / 2);
        disc(0, 0);
    }
}

difference()
{
    union()
    {
        bosses(1);
        frame();
    }
    
    bosses(0);
    
    translate([0, -(5.5 / 4) * quarterInch, -(anchorDepth )])
        rotate(a = [-30, 0, 0])
            cube([inch * 2, quarterInch, 2 * inch], center = true);
    
    translate([0, postSpacing + ((5.5 / 4) * quarterInch), -(anchorDepth )])
        rotate(a = [30, 0, 0])
            cube([inch * 2, quarterInch, 2 * inch], center = true);   
}

/*
linear_extrude(height = quarterInch, center = true)
polygon(
    points = 
    [
        [0, 0],
        [-halfSpan, holeSpacing / 2],
        [0, holeSpacing],
        [halfSpan, holeSpacing / 2],
    ],
    paths = [[0, 1, 2, 3]]);*/