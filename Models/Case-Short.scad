// Dimensions
totalWidth = 190.5; // 7.5"
totalLength = 190.5;
inch = 25.4;
halfInch = inch / 2;
quarterInch = inch / 4; // .25"
floorDepth = quarterInch;
edgeWidth = quarterInch;
bossHeight = quarterInch;
cavityDepth = .5 * inch;
boltRadius = 1.4;
dividerHeight = 56;
dividerDepth = inch;

// Boss
module boss(x, y)
{
    translate ([x, y, floorDepth])
    {
        difference()
        {
            cylinder(h = bossHeight + floorDepth, r = quarterInch, center = true);
            cylinder(h = bossHeight + floorDepth + 1, r = boltRadius, center = true);
        }
    }
}

difference()
{
    union()
    {
        // Box
        difference()
        {
            cube([totalWidth, totalLength, floorDepth + cavityDepth], false);

            // Hollow out
            translate([edgeWidth, edgeWidth, floorDepth])
            {
                cube([
                    totalWidth - (2 * edgeWidth),
                    totalLength - (2 * edgeWidth),
                    cavityDepth + 1], 
                    false);
            }
            
            // PLX holes
            translate([
                totalWidth / 4, 
                dividerHeight / 2 + 5, 
                floorDepth / 2])
            {
                cube([totalWidth / 3, dividerHeight * .75, floorDepth + 1], true);
            }
            
            // Between PLX and bus
            translate([
                totalWidth / 2, 
                inch * 3, 
                floorDepth / 2])
            {
                cube([totalWidth * 0.9, inch, floorDepth + 1], true);
            }
            
            // Right circle
            translate([
                inch * 6.3, 
                inch * 5.9, 
                floorDepth / 2])
            {
                cylinder(h = floorDepth + 1, r = inch * .95, center = true);
            }

            // Left hole
            translate([
                inch * 1.25, 
                inch * 5.55, 
                floorDepth / 2])
            {
                cube([inch, inch * 3, inch * 3], true);
            }

            translate([
                inch * 2.25, 
                inch * 5, 
                floorDepth / 2])
            {
                cube([inch * 1.5, inch * 1.5, inch * 3], true);
            }

            translate([
                inch * 4.5, 
                inch * 5.75, 
                floorDepth / 2])
            {
                cube([inch * 1.4, inch * 2.5, inch * 3], true);
            }

            // Right small circle
            translate([
                inch * 2.75, 
                inch * 5.5, 
                floorDepth / 2])
            {
                //cylinder(h = floorDepth + 1, r = inch * .75, center = true);
            }

            translate([
                inch * 3.8, 
                inch * 5.95, 
                floorDepth / 2])
            {
                cube([inch * 2.7, inch * 1.3, inch * 3], true);
            }

            translate([
                totalWidth * 3 / 4, 
                dividerHeight / 2 + 5, 
                floorDepth / 2])
            {
                cube([totalWidth / 3, dividerHeight * .75, floorDepth + 1], true);
            }

            // Wiring passage
            translate([
                totalWidth - (edgeWidth / 2), 
                totalLength / 2, 
                floorDepth * 1.5 + 1 + cavityDepth / 2])
            {
                cube([edgeWidth + 1, inch, cavityDepth * .85], center = true);
                /*
                rotate(a = -90, v = [0, 1, ])
                linear_extrude(height = inch, center=true, convexity = 1, twist = 0)
                polygon(
                    points =
                    [
                        [-halfInch, -halfInch],
                        [-halfInch, halfInch],
                        [halfInch, 0]
                    ],
                    paths =
                    [
                        [0, 1, 2]
                    ]
                );*/
                
            }    
        }  

        // Arduino
        translate([88 - 14, 125, 0])
        {
            boss (14, 2.5); //  lower left
            boss (12.50,               2.5 + 5.1 + 27.9 + 15.2); // top left
            boss (14.05 + 50.8,        2.5 + 5.1); // lower Uno hole
            boss (14.05 + 50.8,        2.5 + 5.1 + 27.9); // upper Uno hole
            boss (14.05 + 50.8 + 24.1, 2.5 + 5.1 + 27.9 + 15.2); // upper Uno hole
            boss (12.00 + 82.65,       2.5); // lower right
        }

        // Voltage regulator
        translate([2 * quarterInch, 153, 0])
        {
            boss (0, 0); //  left
            boss (41.275, 0); //  right
        }

        // PLX divider
        translate([totalWidth / 2, dividerHeight, floorDepth + dividerDepth / 2])
        {
            cylinder(h = inch + 1, r = quarterInch, center = true);
        }

        translate([totalWidth / 2, dividerHeight / 2, floorDepth + dividerDepth / 2])
        {
            difference()
            {
                cube([halfInch, dividerHeight , dividerDepth + 1], center = true);

                translate([0, halfInch + quarterInch, 0])
                {
                    cylinder(h = dividerHeight + floorDepth + 1, r = boltRadius, center = true);
                }
                
                translate([0, (-halfInch) + quarterInch, 0])
                {
                    cylinder(h = dividerHeight + floorDepth + 1, r = boltRadius, center = true);
                }
            }
        }


        // Wiring anchors
        translate([0, 96, 0])
        {
            boss(inch, 0);
            boss(inch + inch, 0);
            
            translate([0, quarterInch, 0])
            {
                boss(inch + inch + inch, 0);
                boss(inch + inch + inch + inch, 0);
                boss(inch + inch + inch + inch + inch, 0);
                boss(inch + inch + inch + inch + inch + inch, 0);
                boss(inch + inch + inch + inch + inch + inch + inch, 0);
            }
        }
    }
    
    // Remove lower half
    translate([
        totalWidth / 2, 
        0, 
        floorDepth / 2])
    {
        cube([totalWidth + 2, 8.6 * inch, inch * 3], true);
    }
}
