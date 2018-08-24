// Dimensions
totalWidth = 190.5; // 7.5"
totalLength = 190.5;
inch = 25.4;
halfInch = inch / 2;
quarterInch = inch / 4; // .25"
floorDepth = 3;   // Real = .25 inches
edgeWidth = quarterInch;
bossHeight = 3;
cavityDepth = 0.1 * inch;       // Real = 2 inches
boltRadius = 1.4;
dividerHeight = 56;
dividerDepth = inch;
bossRadius = 3;

arduinoHeight = floorDepth + (bossHeight / 2) - 1;

    lowerLeftX = 14.25;
    upperLeftX = 16;
    upperRightX = 14.05 + 52.5 + 24.1;
    lowerRightX = 12.00 + 85.3;
    upperY = 2.5 + 5.1 + 27.9 + 15.2;
    lowerY = 2.5;
    unoX = 14 + 52.5;

// Boss
module boss(x, y, solid)
{
    translate ([x, y, 0])
    {
        if (solid > 0)
        {
            cylinder(h = bossHeight, r = bossRadius, center = true, $fn = 20);
        }
        else
        {
            cylinder(h = bossHeight + 30, r = boltRadius, center = true, $fn = 10);
        }
    }
}

module arduinoDue(solid)
{   
    translate([0, 0, (bossHeight / 2) - 1])
    {
        boss (lowerLeftX,          lowerY,                    solid); // lower left
        boss (16,                  upperY,                    solid); // upper left
        boss (unoX,                lowerY + 5.1,              solid); // lower Uno hole
        boss (unoX,                lowerY + 5.1 + 27.9,       solid); // upper Uno hole
        boss (upperRightX,         upperY,                    solid); // upper right
        boss (lowerRightX,         lowerY,                    solid); // lower right
        
        if (solid == 0)
        {
            // Lower left
            translate([lowerLeftX - 10, lowerY , (bossHeight / 2) - 2])
            {
                cube ([10, 10, 3]);
            }
            
            // Top two
            translate([upperLeftX, upperY - 1, (bossHeight / 2) - 2])
            {
                cube ([upperRightX - upperLeftX + 10, 2, 3]);
            }
            
            // Bottom right two
            translate([lowerRightX - (inch * 2) - 1, lowerY - 1, (bossHeight / 2) - 2])
            {
                cube ([inch * 2, 2, 3]);
            }  
  
            translate([lowerRightX - 1, lowerY, (bossHeight / 2) - 2])
            {
                cube ([2, 20, 3]);
            }  

            translate([lowerRightX - 3, lowerY, (bossHeight / 2) - 2])
            {
                cube ([2, 20, 3]);
            }  

            // Upper Uno hole
            translate([unoX - 1, lowerY + 23, (bossHeight / 2) - 2])
            {
                cube ([2, 10, 3]);
            }  
        }
    }
}

buffer = 5;
xOffset = 0;
yOffset = lowerY + (buffer * 1.5);
platformHeight = upperY - lowerY + (5 * buffer);
mountingHoleWidth = lowerRightX;
mountingHoleHeight = platformHeight - (2 * buffer);
echo("mountingHoleWidth = ", mountingHoleWidth);
echo("mountingHoleHeight = ", mountingHoleHeight);

module mountingHoles()
{
    translate([buffer, buffer, 0])
    {
        cylinder(r = boltRadius + .3, h = 20, center = true, $fn = 20);
    }
    
    translate([mountingHoleWidth, buffer, 0])
    {
        cylinder(r = boltRadius + .3, h = 20, center = true, $fn = 20);
    }

    translate([(mountingHoleWidth + buffer) / 2, buffer, 0])
    {
        cylinder(r = boltRadius + .3, h = 20, center = true, $fn = 20);
    }
}

difference()
{
    union()
    {
        cube([lowerRightX + buffer, platformHeight, floorDepth], false);

        // Arduino positive
        translate([xOffset, yOffset, floorDepth])
        {
            arduinoDue(1);
        }
    }

    // Arduino negative
    translate([xOffset, yOffset, floorDepth])
    {
        arduinoDue(0);
                
        translate ([lowerLeftX - buffer, lowerY + buffer, -10])
        {
            cube ([unoX - lowerLeftX - buffer, upperY - lowerY - (2 * buffer), inch], false);
        }
            
        translate ([unoX + (2 * buffer), lowerY + buffer, -10])
        {
            cube ([lowerRightX - unoX - (2 * buffer), upperY - lowerY - (2 * buffer), inch], false);
        }
        
    }

        mountingHoles();
        
        translate([0, mountingHoleHeight, 0])
        {
            mountingHoles();
        }

}
