#include <testlib/testlib_test.h>
#include <volm/conf/volm_conf_query.h>
#include <vul/vul_file.h>
#include <vsph/vsph_spherical_coord.h>
#include <vsph/vsph_sph_point_3d.h>
#include <bpgl/bpgl_camera_utils.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vsol/vsol_point_2d.h>
#include <vsol/vsol_polygon_2d.h>
#include <bpgl/depth_map/depth_map_region.h>
#include <bpgl/depth_map/depth_map_region_sptr.h>
#include <bpgl/depth_map/depth_map_scene.h>
#include <bpgl/depth_map/depth_map_scene_sptr.h>


static void test_volm_conf_query()
{

  // create a depth_map_scene
  depth_map_scene_sptr depth_scene = new depth_map_scene;
  unsigned ni = 1280;
  unsigned nj = 720;
  depth_scene->set_image_size(ni, nj);
  // add a sky object
  vsol_point_2d_sptr ps0 = new vsol_point_2d(0.0, 200.0);
  vsol_point_2d_sptr ps1 = new vsol_point_2d(1280.0, 200.0);
  vsol_point_2d_sptr ps2 = new vsol_point_2d(1280.0, 0.0);
  vsol_point_2d_sptr ps3 = new vsol_point_2d(0.0, 0.0);
  vcl_vector<vsol_point_2d_sptr> verts_sky;
  verts_sky.push_back(ps0);  verts_sky.push_back(ps1);
  verts_sky.push_back(ps2);  verts_sky.push_back(ps3);
  vsol_polygon_2d_sptr sp = new vsol_polygon_2d(verts_sky);
  depth_scene->add_sky(sp, 255, "sky");
  // add a ground plane object
  vsol_point_2d_sptr p0= new vsol_point_2d(0.0, 720.0);
  vsol_point_2d_sptr p1= new vsol_point_2d(1280.0, 720.0);
  vsol_point_2d_sptr p2= new vsol_point_2d(1280.0, 500.0);
  vsol_point_2d_sptr p3= new vsol_point_2d(0.0, 500.0);
  vcl_vector<vsol_point_2d_sptr> verts;
  verts.push_back(p0);   verts.push_back(p1);
  verts.push_back(p2);   verts.push_back(p3);
  //vsol_polygon_2d_sptr gp = new vsol_polygon_2d(verts);
  //depth_scene->add_ground(gp, 0.0, 0.0, 0, "beach", 6);
  // add an object
  vsol_point_2d_sptr pb0 = new vsol_point_2d(640.0, 400.0);
  vsol_point_2d_sptr pb1 = new vsol_point_2d(940.0, 600.0);
  vsol_point_2d_sptr pb2 = new vsol_point_2d(340.0, 200.0);
  vcl_vector<vsol_point_2d_sptr> verts_bd;
  verts_bd.push_back(pb0);  verts_bd.push_back(pb1);  verts_bd.push_back(pb2);
  vsol_polygon_2d_sptr bp = new vsol_polygon_2d(verts_bd);
  vgl_vector_3d<double> np(1.0, 1.0, 0.0);
  depth_scene->add_region(bp, np, 100.0, 1000.0, "hotel", depth_map_region::FRONT_PARALLEL, 1, 15);
  depth_scene->scene_regions()[0]->set_ref(true);
  unsigned num_of_ref_objects = 1;
  vcl_string dms_bin_file = "./depth_map_scene.bin";
  vsl_b_ofstream ofs_dms(dms_bin_file);
  depth_scene->b_write(ofs_dms);
  ofs_dms.close();

  // create a camera space
  double head_mid=180.0,       head_radius=0.0,  head_inc=2.0;
  double tilt_mid=96.967085,   tilt_radius=5.0,  tilt_inc=2.0;
  double roll_mid=0.5632510,   roll_radius=0.0,  roll_inc=0.0;
  //double top_fov_vals[] = {3.0,  4.0, 5.0, 12.0, 17.0, 18.0,19.0, 20.0, 24.0};
  double top_fov_vals[] = {14.538983, 12.538983, 16.538983};
  vcl_vector<double> fovs;
  fovs.push_back(top_fov_vals[0]);


  double altitude = 1.4;
  volm_camera_space_sptr csp = new volm_camera_space(fovs, altitude, ni, nj,
                                                     head_mid, head_radius, head_inc,
                                                     tilt_mid, tilt_radius, tilt_inc,
                                                     roll_mid, roll_radius, roll_inc);

  if (depth_scene->ground_plane().size() > 0)  // enforce ground plane constraint if user specified a ground plane
  {
    camera_space_iterator cit = csp->begin();
    for ( ; cit != csp->end(); ++cit) {
      unsigned current = csp->cam_index();
      vpgl_perspective_camera<double> cam = csp->camera(); // camera at current state of iterator
      bool success = true;
      for (unsigned i = 0; success && i < depth_scene->ground_plane().size(); i++)
        success = depth_scene->ground_plane()[i]->region_ground_2d_to_3d(cam);
      if (success) // add this camera
        csp->add_camera_index(current);
    }
  }
  else
    csp->generate_full_camera_index_space();

  // construct a volm_conf_queue
  volm_conf_query_sptr query = new volm_conf_query(csp, depth_scene, 25);

  vcl_cout << "configurational query reference object list: ";
  vcl_vector<vcl_string> ref_object_names = query->ref_obj_name();
  for (vcl_vector<vcl_string>::iterator vit = ref_object_names.begin(); vit != ref_object_names.end(); ++vit)
    vcl_cout << (*vit) << ", ";
  vcl_cout << vcl_endl;

  vcl_cout << "configurational query camera list: " << vcl_endl;
  vcl_vector<vcl_string> cam_string = query->camera_strings();
  for (vcl_vector<vcl_string>::iterator vit = cam_string.begin();  vit != cam_string.end();  ++vit)
    vcl_cout << "\t" << (*vit) << vcl_endl;
  vcl_cout << query->cameras().size() << " perspective cameras are created." << vcl_endl;
  TEST("configurational query cameras", query->cameras().size(), query->camera_angles().size());
  TEST("configurational query reference objects", query->ref_obj_name().size(), num_of_ref_objects);

  vcl_cout << "configurational query has following reference object: " << vcl_endl;
  vcl_vector<vcl_string> query_ref_obj = query->ref_obj_name();
  for (vcl_vector<vcl_string>::iterator vit = query_ref_obj.begin();  vit != query_ref_obj.end();  ++vit)
    vcl_cout << " " << (*vit);
  vcl_cout << vcl_endl;

  vcl_cout << "configurational query has following configuration object: " << vcl_endl;
  vcl_vector<vcl_map<vcl_string, volm_conf_object_sptr> > conf_objs = query->conf_objects();
  vcl_vector<vcl_map<vcl_string, vcl_pair<float, float> > > d_tol = query->conf_objects_d_tol();
  for (unsigned i = 0; i < conf_objs.size(); i++) {
    vcl_cout << "\t camera: " << cam_string[i] << vcl_endl;
    for (vcl_map<vcl_string, volm_conf_object_sptr>::iterator mit = conf_objs[i].begin();  mit != conf_objs[i].end(); ++mit) {
      vcl_cout << "\t\t obj name: " << mit->first << "\t\t" ;  mit->second->print(vcl_cout);
      vcl_cout << "\t\t  distance tolerance: " << d_tol[i][mit->first].first << " to " << d_tol[i][mit->first].second << vcl_endl;
    }
  }

  return;
}

TESTMAIN( test_volm_conf_query );