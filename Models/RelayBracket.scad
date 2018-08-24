topRadius = 18;
bottomRadius = 12;

topPinTopRadius = 3.3;
topPinMiddleRadius = 4.4;
topPinDepth = 15;

sidePinTopRadius = 3.4;
sidePinMiddleRadius = 4;
sidePinDepth = 7.5;
relayWidth = 20;
relayOffset = 25;
thickness = 20;

coreHeight = relayWidth;
spacerHeight = 4;

$fn = 50;

difference() 
{
    union()
    {
        cylinder(r1 = bottomRadius, r2 = topRadius, h = relayWidth, center = true);
        
        translate([0, 0, (coreHeight / 2) + (spacerHeight / 2)])
            cylinder(r = topRadius, h = spacerHeight, center = true);
        
        translate([relayOffset - (thickness / 2), 0, 0])
        {
            cube([thickness, relayWidth, relayWidth], center = true);
        }
        
    }
    
    translate([0, 0, (coreHeight / 2) - (topPinDepth / 2) + spacerHeight])
    {
        cylinder(r1 = topPinMiddleRadius, r2 = topPinTopRadius, h = topPinDepth + 1, center = true);
    }
    
    translate([0, 0, (coreHeight / 2) - (topPinDepth) - 10 + spacerHeight])
    {
        cylinder(r = topPinMiddleRadius, h = 20, center = true);
    }
    
    rotate(a = 90, v = [0, 1, 0])
    {
        translate([0, 0, relayOffset])
        {
            translate([0, 0, -(sidePinDepth / 2)])
            {
                cylinder (r1 = sidePinMiddleRadius, r2 = sidePinTopRadius, h = sidePinDepth + .1, center = true);
            }
            
            translate([0, 0, -sidePinDepth - 6])
            {
                cylinder (r = sidePinMiddleRadius, h = 12, center = true);
            }
            
        }
        
            bridgeWidth = 4.5;
            translate([-(sidePinMiddleRadius - bridgeWidth / 2), 0, relayOffset - (sidePinDepth / 2)])
            {
                cube([bridgeWidth, bridgeWidth, sidePinDepth + 12], center = true);
            }
        
    }
}