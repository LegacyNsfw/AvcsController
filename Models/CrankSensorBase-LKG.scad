width = 40;
height = 30;
baseDepth = 5;
lidDepth = 10;
ridgeTopWidth = 7;
ridgeBottomWidth = 10;
ridgeSpacing = 15;
ridgeHeight = 1.4;
mainHoleSpacing = 20;
thirdHoleSpacing = 17;
mainHoleDistanceFromEdge = 6;

sensorHoleCenter = 12.5; 
sensorWidth = 6.5;
sensorHeight = 11;

pulleyRadius = 48;
pulleyX = 41;
pulleyY = 40;

$fn = 20;

delta = 0;

module mountingHole(lid)
{
    cylinder(r = 1.6, h = 100, center = true);
    
    if (lid)
    {
        delta = 2.001;
    }
    else
    {
        delta = 1.999;
    }
    
    translate([0, 0, baseDepth + delta])
        cylinder(r = 3.4, h = 4, $fn = 6, center = true);
}

module holes(lid)
{
    translate([-17.5, 0, 0])
    {
        translate([mainHoleDistanceFromEdge, mainHoleSpacing / 2, 0])
        {
            mountingHole(lid);
        }

        translate([mainHoleDistanceFromEdge + thirdHoleSpacing, mainHoleSpacing / 2, 0])
        {
            mountingHole(lid);
        }

        translate([mainHoleDistanceFromEdge, -mainHoleSpacing / 2, 0])
        {
            mountingHole(lid);
        }
    }
}

module ridge()
{
    translate([-25, 0, -0.001])
    {
        rotate([0, 0, 90])
        rotate([90, 0 ,0])
        {
            linear_extrude(width + 10)
            {
                ridgeProfile =
                [
                    [-ridgeBottomWidth / 2, 0], 
                    [ridgeBottomWidth / 2, 0], 
                    [ridgeTopWidth / 2, ridgeHeight], 
                    [-ridgeTopWidth / 2, ridgeHeight]
                ];
                
                polygon(points=ridgeProfile);
            }
            //cube([width * 3, ridgeWidth, ridgeDepth], center=true);
        }
    }
}

module pinTrench()
{
    translate ([-14.5, 0, baseDepth])
    {
        cube([4, 10, 4], center = true);
    }
}

module ridges()
{
    translate([0, ridgeSpacing / 2, 0])
    {
        ridge();
    }

    translate([0, -ridgeSpacing / 2, 0])
    {
        ridge();
    }
}

module pulley()
{
    translate([pulleyX, -pulleyY, (-baseDepth/2) +2])
    {
        cylinder(r1 = pulleyRadius , r2 = pulleyRadius - 5, h = baseDepth+1, $fn=200);
        cylinder(r = pulleyRadius -4.5, h = baseDepth + lidDepth + 20, $fn=200);
    }
}

module base()
{
    difference()
    {
        union()
        {
            //pinTrench();            
            //pulley();
            
            difference()
            {
                translate([0, 0, baseDepth / 2])
                {
                    cube([width, height, baseDepth], center = true);
                }
                holes(false);
            }
        }
        
        pinTrench();      // 
        ridges();//
        
        translate([sensorHoleCenter, 0, 10])//
        {
            cube([sensorWidth, sensorHeight, 40], center = true);
        }
        
        pulley();//
    }
}

module lid()
{
    difference()
    {
        union()
        {   
            difference()
            {
                translate([0, 0, (lidDepth / 2) + baseDepth])
                {
                    cube([width, height, lidDepth], center = true);
                }
                holes(true);
            }
        }

        rightEdge = sensorHoleCenter + sensorWidth / 2;
        boardWidth = 33;
        position = rightEdge - (boardWidth / 2);
        translate([position, 0, 8.4])
        {
            cube([boardWidth, 14.5, 7], center = true);
        }

        translate([-width / 2, 0, baseDepth + 2])
        {
            cube([25, 10, 14], center = true);
        }

        pulley();
    }
}

//ridges();
//base();
lid();