#include <testlib/testlib_test.h>
#include <vcl_iostream.h>

#include <bpgl/bpgl_camera_utils.h>
#include <vgl/vgl_vector_3d.h>
#include <vgl/vgl_polygon.h>
static void test_camera_utils()
{
  // =============  test kml camera constructor========================
  unsigned ni = 1280, nj = 720;
  double nid = static_cast<double>(ni), njd = static_cast<double>(nj);
  double right_fov = 29.605;
  double top_fov = 17.725;
  double altitude = 1.6;
  double heading = 170.65;
  double tilt = 78.62;
  double roll = -2.76;
  vpgl_perspective_camera<double> cam =
    bpgl_camera_utils::camera_from_kml(nid, njd, right_fov, top_fov,
                                       altitude, heading, tilt, roll);
  vgl_vector_3d<double> pray = cam.principal_axis();
  //s,c == sin,cos(head), st == sin(tilt)
  double s = 0.1625, c = -0.9867, st = 0.9803;
  double dif = vcl_fabs(pray.x()-(s*st));
  dif += vcl_fabs(pray.y()-(c*st));
  TEST_NEAR("principal ray (KML)", dif, 0.0, 0.0001);

  // ==============   test ray direction on unit sphere ======
  double elevation, azimuth;
  bpgl_camera_utils::ray_spherical_coordinates(cam, 640.0, 360.0, elevation,
					       azimuth, "degrees");
  double er = vcl_fabs(elevation - (180.0-tilt));
  er += vcl_fabs(azimuth - (90.0 -heading));
  TEST_NEAR("ray spherical coordinates", er, 0.0, 0.001);

  // ================= test project poly onto sphere =======
  // remove roll for poly angle tests
  roll = 0.0;
  vpgl_perspective_camera<double> cam0 =
    bpgl_camera_utils::camera_from_kml(nid, njd, right_fov, top_fov,
                                       altitude, heading, tilt, roll);  
  
  vgl_point_2d<double> p0(640.0, 0.0), p1(1280.0, 0.0);
  vgl_point_2d<double> p2(1280.0, 360.0), p3(640.0, 360.0);//<==principal pt
  vcl_vector<vgl_point_2d<double> > sheet;
  sheet.push_back(p0);  sheet.push_back(p1);
  sheet.push_back(p2);  sheet.push_back(p3);
  vgl_polygon<double> image_poly, sph_poly;
  image_poly.push_back(sheet);
  vcl_cout << "in image\n" << image_poly << '\n';
  sph_poly = 
    bpgl_camera_utils::project_poly_onto_unit_sphere(cam0, image_poly, "degrees");
  vcl_cout << "on sphere\n" << sph_poly << '\n';
  vgl_point_2d<double> sp0 = sph_poly[0][0];
  vgl_point_2d<double> sp2 = sph_poly[0][2];
  vgl_point_2d<double> sp3 = sph_poly[0][3];
  //x is elevation and y is azimuth
  double t_fov = vcl_fabs(sp0.x()-sp3.x());
  double r_fov = vcl_fabs(sp2.y()-sp3.y());
  // shouldn't expect exact recovery since focal length is 
  // the average of top and right fov
  er = vcl_fabs(t_fov-top_fov) + vcl_fabs(r_fov-right_fov);
  TEST_NEAR("spherical polygon ", er, 0.0, 1.0);//1 degree
}


TESTMAIN(test_camera_utils);
