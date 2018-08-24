// TO DO:
//
// Remove lid screws that will interfere with PLX
// Add back the rest of the lid screws
// Make the rest of the bosses go through the floor
// Make the wiring notch open all the way to the top
// Make the wiring notch extend down to the top of the nearest boss
// Remove 2 of the 5 wiring bosses

// Dimensions
totalWidth = 190.5; // 7.5"
totalLength = 199;
inch = 25.4;
halfInch = inch / 2;
quarterInch = inch / 4; // .25"
floorDepth = quarterInch / 2;  
edgeWidth = quarterInch;
bossHeight = quarterInch / 2;
cavityDepth = (2 * inch) - (2 * floorDepth);
boltRadius = 1.4;
dividerHeight = 80; // 56
dividerDepth = inch;
arduinoX = 29.5;
arduinoY = totalLength - (65 + (2 * edgeWidth));
lidThickness = quarterInch / 2;
lidScrewHoleDepth = 10; // 12.5;
lidScrewLidBoltRadius = boltRadius + 0.5; 
lidScrewCaseBoltRadius = boltRadius; 
lidScrewMagnetRadius = boltRadius; 
centerSupportY = 90;

plxHeight = 57;
plxEdgeWidth = 3;
plxEdgeHeight = 4.5;

module plxPost(x, y, solid)
{
    translate([x, y, dividerDepth / 2])
    {
        if (solid)
        {
            cylinder(h = dividerDepth, r = quarterInch, center = true);
        }
        else
        {
            cylinder(h = dividerDepth + 1, r = boltRadius, center = true);
        }
    }
}

module plxBlock(x, y, l)
{
    translate([x, y, dividerDepth / 2])
    {
        cube([halfInch, l, dividerDepth], center = true); 
    }
}


// Boss
module boss(x, y, solid)
{
    translate ([x, y, floorDepth])
    {
        if (solid)
        {
            cylinder(h = bossHeight + floorDepth, r = 3.25, center = true);
        }
        else
        {
            cylinder(h = inch * 2, r = boltRadius, center = true);
        }
    }
}

module anchor(x, y)
{
    translate([x - 5, y, floorDepth])
    {
        cube([5, 10, 6], center = true);
    }
    
    translate([x + 5, y, floorDepth])
    {
        cube([5, 10, 6], center = true);
    }

    translate([x, y, floorDepth + 4])
    {
        cube([15, 10, 2], center = true);
    }
}

plxDeviceHeight = 33;
lidScrewHeight = cavityDepth + floorDepth - plxDeviceHeight;
lidScrewZ = cavityDepth + floorDepth - (lidScrewHeight / 2);

module lidScrew(x, y, positive, lid)
{
    if (positive)
    {
        if (lid)
        {
            translate([x, y, (cavityDepth / 2) + floorDepth])
                cylinder(h = cavityDepth, r1 = 1, r2 = quarterInch + 1, , center = true);
        }
        else
        {
            translate([x, y, lidScrewZ])
                cylinder(h = lidScrewHeight, r1 = 1, r2 = quarterInch, , center = true);
        }
    }
    else
    {
        if (lid)
        {
            translate([x, y, cavityDepth])
                 cylinder(h = lidScrewHoleDepth * 2, r = lidScrewLidBoltRadius, center = true, , $fn = 20);

        }
        else
        {
            translate([x, y, lidScrewZ + 1])
                cylinder(h = lidScrewHeight, r = lidScrewCaseBoltRadius, center = true, $fn = 5);

        }
    }
}

module lidScrews(positive, lid)
{
    lidScrew(edgeWidth, edgeWidth, positive, lid);
    lidScrew(totalWidth - edgeWidth, edgeWidth, positive, lid);
    lidScrew(edgeWidth, totalLength - edgeWidth, positive, lid);
    lidScrew(totalWidth - edgeWidth, totalLength - edgeWidth, positive, lid);
    
    //lidScrew(totalWidth / 2, totalLength - edgeWidth, positive);
    lidScrew(3 * inch, totalLength - edgeWidth, positive, lid);
    lidScrew(totalWidth - (3 * inch), totalLength - edgeWidth, positive, lid);
    
    lidScrew(totalWidth / 2, edgeWidth, positive, lid);
    
    lidScrew(edgeWidth, 3 * inch, positive, lid);
    lidScrew(totalWidth - edgeWidth, 3 * inch, positive, lid);

    lidScrew(edgeWidth, totalLength - (3 * inch), positive, lid);
    lidScrew(totalWidth - edgeWidth, totalLength - (3 * inch), positive, lid);
}

arduinoHolesWidth = 92;
arduinoHolesHeight = 63;

module arduinoBossesSingleRow(solid)
{
    boss(0, 0, solid);
    boss(arduinoHolesWidth, 0, solid);
    boss(arduinoHolesWidth / 2, 0, solid);
}

module arduinoDue(solid)
{
    arduinoBossesSingleRow(solid);
    
    translate([0, arduinoHolesHeight, 0])
    {
        arduinoBossesSingleRow(solid);
    }
}

module bosses(solid)
{
/*    // Voltage regulator
    translate([2 * quarterInch, 175, 0])
    {
        boss (0, 0, solid); //  left
        boss (41.275, 0, solid); //  right
    }*/

    // Future expansion
    translate([135, 174, 0])
    {
        boss (5, 0, solid); //  left
        boss (35, 0, solid); //  right
    }

    translate([135, 124, 0])
    {
        boss (5, 0, solid); //  left
        boss (35, 0, solid); //  right
    }
    
    if (solid)
    {
        // Wiring anchors
        translate([0, totalLength / 2, 0])
        {           
            anchor(20, 0);
            anchor(totalWidth / 2 - 25, 0);
            anchor(totalWidth / 2 + 25, 0);
            anchor(totalWidth - 20, 0);
        }
    }
    
    boss(totalWidth / 2, 25.5, solid);
    boss(totalWidth / 2, 66.5, solid);
    
    boss(totalWidth / 2 - 11, totalLength / 2 - 15, solid);
    boss(totalWidth / 2 + 11, totalLength / 2 - 15, solid);    
}

module wiringPassage()
{
    // Wiring passage
    translate([
        totalWidth - (edgeWidth / 2), 
        totalLength / 2, 
        30])
    {
        cube([edgeWidth + 1, inch / 2, cavityDepth ], center = true);
    }    
}

module box()
{
    difference()
    {
        union()
        {
            lidScrews(true, false);
            
            // Box
            difference()
            {
                union()
                {
                    cube([totalWidth, totalLength, floorDepth + cavityDepth], false);
                }

                // Hollow out
                translate([edgeWidth, edgeWidth, floorDepth])
                {
                    cube([
                        totalWidth - (2 * edgeWidth),
                        totalLength - (2 * edgeWidth),
                        cavityDepth + 1], 
                        false);
                }
                wiringPassage();
            }  

            bosses(true);
            
            // Arduino
            translate([arduinoX, arduinoY, 0])
            {
                //arduinoDue_Old();
                arduinoDue(1);
            }

            // Center support
            translate([totalWidth / 2, centerSupportY, floorDepth + cavityDepth / 2])
            {
                difference()
                {
                    cylinder(h = cavityDepth, r = quarterInch, center = true);
                    cylinder(h = cavityDepth + 1, r = boltRadius, center = true);
                }
            }
            
            // PLX edges
            translate([totalWidth / 4, edgeWidth + plxHeight + plxEdgeHeight / 2, floorDepth + plxEdgeHeight / 2])
            {
                cube ([2 * inch, plxEdgeWidth, plxEdgeHeight], center = true);
            }

            translate([3 * totalWidth / 4, edgeWidth + plxHeight + plxEdgeHeight / 2, floorDepth + plxEdgeHeight / 2])
            {
                cube ([2 * inch, plxEdgeWidth, plxEdgeHeight], center = true);
            }
                       
            difference()
            {
                union()
                {
                    plxBlock(totalWidth / 2, 7, 14);
                    plxPost(totalWidth / 2, 14, true);
            
                    plxPost(totalWidth / 2, 78, true);
                    plxBlock(totalWidth / 2, 84, 12);
                }
                
                plxPost(totalWidth / 2, 14, false);            
                plxPost(totalWidth / 2, 78, false);
            }
        }
        
        translate([arduinoX, arduinoY, 0])
        {
            arduinoDue(0);
        }
        
        lidScrews(false, false);
        
        bosses(false);

    /*    
        // Remove lower half
        translate([
            totalWidth / 2, 
            0, 
            floorDepth / 2])
        {
            cube([totalWidth + 2, 8.6 * inch, inch * 3], true);
        }*/
    }
}

lidRimWidth = 1.5 * edgeWidth;

module duct(ductWidth, ductHeight, thickness, scale, depth, positive)
{
    if (positive)
    {
        linear_extrude(height = depth, center = true, scale = scale)
            square([ductWidth, ductHeight], center = true);
    }
    else
    {
        translate([0, 0, -0.01])
        {
            linear_extrude(height = depth + 0.03, center = true, scale = scale)
                square([ductWidth - thickness, ductHeight - thickness], center = true);
        }
    }
}


module lid()
{
    difference()
    {
        union()
        {
            // button access
            translate([85, 125, floorDepth+cavityDepth - 1])
            {
                //duct(45, 20, 2, [1.1, 1.3], 10, true);
            }

            // screen access
            translate([101, 150, floorDepth+cavityDepth + 2])
            {
                
                //duct(80, 35, 5, [1.1, 1.3], 4, true);
            }

            difference()
            {
                union()
                {        
                    translate([0.01, 0.01, (floorDepth + cavityDepth) - lidThickness])
                    {
                        cube([totalWidth - 0.02, totalLength - 0.02, lidThickness + 4], false);
                    }
                }
                
                union()
                {
                    translate([lidRimWidth, lidRimWidth, (cavityDepth - 1)])
                    {
                        cube([totalWidth - 2 * lidRimWidth, totalLength - 2 * lidRimWidth, lidThickness + 1], false);
                    }
                                
                    box();
                    lidScrews(true , true);            
                    translate([0, 0, 0.5 * inch])
                    {
                        lidScrews(false, true);
                    }
                    
                    // center support
                    lidScrew(totalWidth / 2, centerSupportY, 0, 1);
                }
            }
        }
        
        // button access
        translate([16, 124, floorDepth+cavityDepth - 3])
        {
            cube([58.5-16, 137-124, 25], false);
        }
    
        // screen access
        translate([18, 143, floorDepth+cavityDepth - 3])
        {
            cube([92-18, 168-142, 20], false);
        }        
    }
}

module cornerPreview()
{
    difference()
    {
        //box();
        lid();
        union()
        {
            translate([-1, 20, -1])
            {
                cube([12 * inch, 12 * inch, 4 * inch], false);
            }
            
            translate([20, -1, -1])
            {
                cube([12 * inch, 12 * inch, 4 * inch], false);
            }
            
            translate([-inch / 2, -inch / 2, -6])
            {
                cube([2 * inch, 2 * inch, inch * 1.33], false);
            }
            
        }
    }
}

lid();
//box();

//cornerPreview();
