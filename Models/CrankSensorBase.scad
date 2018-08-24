width = 38;
height = 38;
baseDepth = 6;
lidDepth = 8;
ridgeTopWidth = 7;
ridgeBottomWidth = 10;
ridgeSpacing = 15;
ridgeHeight = 1.5;
mainHoleSpacing = 30;
thirdHoleSpacing = 25;
mountingHoleDistanceFromEdge = 5;

sensorMountingHole = 10;
sensorHoleCenter = 12.5; 
sensorWidth = 6.5;
sensorHeight = 11;

pulleyRadius = 52;
pulleyX = 43;
pulleyY = 43;

$fn = 20;

delta = 0;

module mountingHole(lid)
{
    cylinder(r = 1.6, h = 100, center = true);
    
    if (lid)
    {
        translate([0, 0, baseDepth + 1.999])
            cylinder(r = 3.4, h = 4, $fn = 6, center = true);
    
    }
    else
    {
        translate([0, 0, baseDepth + 2.001])
            cylinder(r = 3.4, h = 4, $fn = 6, center = true);
    }
}

module holes(lid)
{
    translate([-width / 2, 0, 0])
    {
        translate([mountingHoleDistanceFromEdge, mainHoleSpacing / 2, 0])
        {
            mountingHole(lid);
        }

        translate([width - mountingHoleDistanceFromEdge, mainHoleSpacing / 2, 0])
        {
            mountingHole(lid);
        }

        translate([mountingHoleDistanceFromEdge, -mainHoleSpacing / 2, 0])
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
    
    translate([0, -ridgeSpacing - ridgeSpacing / 2, 0])
    {
        ridge();
    }
}

module pinTrench()
{
    translate ([-14.5, 0, baseDepth])
    {
        cube([4, 10, 4], center = true);
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
    
        // Sensor mounting hole    
        translate([-(width / 2) + sensorMountingHole, 0, 1.999])
        {
            cylinder(r = 1.6, h = 100, center = true);
            cylinder(r = 3.4, h = 4, $fn = 6, center = true);
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
            cube([30, 10, 11], center = true);
        }

        pulley();
    }
}

//ridges();
//base();
lid();