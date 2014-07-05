use <jnode_v33.scad>

module jnbbox(extrax=0) { // for jncc3000
  union() {
  translate(jnpos + [0,18,7]) rotate([180,0,0]) cube(size=[51.5,18.5,7]+[0,0,extrax]);
  translate([0,18,4]) rotate([180,0,0]) {
    translate([51.300,1.400,0]) translate([0,0,1.600])  cube(size=[7.603,9.000,2.800]); // USB
    translate([51.950,12.300,0]) translate([0,0,1.600]) cube(size=[6.700,3.700,1.650]); // BUTTON
  }
  translate([15,18,4]) rotate([180,0,0]) {
    translate([28.400,7.750,0]) translate([0,0,-6.000]) cube(size=[1.200,2.000,9.000]); // LED1
    translate([26.975,7.750,0]) translate([0,0,-6.000]) cube(size=[1.200,2.000,9.000]); // LED2
	translate([1.900,12.800,0]) translate([0,0,-6.000]) cube(size=[5.000,3.000,9.000]); // MPL
	translate([21.350,12.900,0]) translate([0,0,-6.000]) cube(size=[3.900,3.900,9.000]); // VCNL
	translate([28.100,11.850,0]) translate([0,0,1.700])  cube(size=[8.000,17.800,2.400]); // BAT
  }
}
}

// complete cube
min_wall_width = 1.2;

module hinge_large(h) {
	translate([batpos[0]-min_wall_width/2,
    	       batpos[1]-min_wall_width/2,
 			   batpos[2]-min_wall_width-h])
	color("blue", .5)
	cube(size=[ 47 + min_wall_width,
    	        18 + 16 + 7 + min_wall_width,
        	    min_wall_width ]);
}

module container(x,y,z) {
	color("blue",.5)  
	minkowski()
	{
	  cube(size=[x,y,z]);
	  sphere(r=1);
	};
}

module hinge(x,y,z) {
    r = z/5.;
	
	rotate([0,0,-90]) 
	difference() {
	minkowski() { cube(size=[x,y,z]);sphere(r=r);	}
	translate([ -(x-x/.6)/2.+min_wall_width/4., y/3./2., -1 ])
	minkowski() { cube(size=[x/4.,2*y/3.,z+2]);sphere(r=r/2.);};
	}
}

jnpos = [4.6,0,-2];
batpos = [7,-24,-2];

// ref only!
//translate([15,18,4]) rotate([180,0,0]) jnode_v33();

color("blue",.5)
difference() {
	union() {
		hinge_large(-3.5);
		translate(jnpos)  container(51.5,18.5,7);
		translate(batpos) container(48,20,6.5);
		translate([10,18+4.2,-.6]) hinge(3,40,4);
		translate([10,-24.1,-.6])  hinge(3,40,4);
	};
	union() {
		// battery bounding box
		translate(batpos + [0,0,-5]) color("gray") cube(size=[48,20,6.5] + [0,0,5]);

		// jnode and bounding box
		translate([0,.5,0]) color("gray",.7) jnbbox(5);
	};
}