/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2013 Blender Foundation.
 * All rights reserved.
 *
 * Original Author: Lukas Toenne
 * Contributor(s): None Yet
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/depsgraph/intern/builder/deg_builder_nodes.h
 *  \ingroup depsgraph
 */

#pragma once

#include "intern/depsgraph_types.h"

struct CacheFile;
struct bGPdata;
struct ListBase;
struct GHash;
struct ID;
struct Image;
struct FCurve;
struct Group;
struct Key;
struct LayerCollection;
struct Main;
struct Material;
struct Mask;
struct MTex;
struct MovieClip;
struct bNodeTree;
struct Object;
struct ParticleSettings;
struct Probe;
struct bPoseChannel;
struct bConstraint;
struct Scene;
struct Tex;
struct World;

struct PropertyRNA;

namespace DEG {

struct Depsgraph;
struct DepsNode;
struct IDDepsNode;
struct TimeSourceDepsNode;
struct ComponentDepsNode;
struct OperationDepsNode;

struct DepsgraphNodeBuilder {
	DepsgraphNodeBuilder(Main *bmain, Depsgraph *graph);
	~DepsgraphNodeBuilder();

	/* For given original ID get ID which is created by CoW system. */
	ID *get_cow_id(const ID *id_orig) const;
	/* Similar to above, but for the cases when there is no ID node we create
	 * one.
	 */
	ID *ensure_cow_id(ID *id_orig);

	/* Helper wrapper function which wraps get_cow_id with a needed type cast. */
	template<typename T>
	T *get_cow_datablock(const T *orig) const {
		return (T *)get_cow_id(&orig->id);
	}

	/* Get fully expanded (ready for use) copy-on-write datablock for the given
	 * original datablock.
	 */
	ID *expand_cow_id(IDDepsNode *id_node);
	ID *expand_cow_id(ID *id_orig);
	template<typename T>
	T *expand_cow_datablock(T *orig) {
		return (T *)expand_cow_id(&orig->id);
	}

	/* For a given COW datablock get corresponding original one. */
	template<typename T>
	T *get_orig_datablock(const T *cow) const {
#ifdef WITH_COPY_ON_WRITE
		return (T *)cow->id.newid;
#else
		return (T *)cow;
#endif
	}

	void begin_build(Main *bmain);

	IDDepsNode *add_id_node(ID *id, bool do_tag = true);
	TimeSourceDepsNode *add_time_source();

	ComponentDepsNode *add_component_node(ID *id,
	                                      eDepsNode_Type comp_type,
	                                      const char *comp_name = "");

	OperationDepsNode *add_operation_node(ComponentDepsNode *comp_node,
	                                      const DepsEvalOperationCb& op,
	                                      eDepsOperation_Code opcode,
	                                      const char *name = "",
	                                      int name_tag = -1);
	OperationDepsNode *add_operation_node(ID *id,
	                                      eDepsNode_Type comp_type,
	                                      const char *comp_name,
	                                      const DepsEvalOperationCb& op,
	                                      eDepsOperation_Code opcode,
	                                      const char *name = "",
	                                      int name_tag = -1);
	OperationDepsNode *add_operation_node(ID *id,
	                                      eDepsNode_Type comp_type,
	                                      const DepsEvalOperationCb& op,
	                                      eDepsOperation_Code opcode,
	                                      const char *name = "",
	                                      int name_tag = -1);

	bool has_operation_node(ID *id,
	                        eDepsNode_Type comp_type,
	                        const char *comp_name,
	                        eDepsOperation_Code opcode,
	                        const char *name = "",
	                        int name_tag = -1);

	OperationDepsNode *find_operation_node(ID *id,
	                                       eDepsNode_Type comp_type,
	                                       const char *comp_name,
	                                       eDepsOperation_Code opcode,
	                                       const char *name = "",
	                                       int name_tag = -1);

	OperationDepsNode *find_operation_node(ID *id,
	                                       eDepsNode_Type comp_type,
	                                       eDepsOperation_Code opcode,
	                                       const char *name = "",
	                                       int name_tag = -1);

	void build_scene(Main *bmain, Scene *scene);
	void build_group(Scene *scene, Group *group);
	void build_object(Scene *scene, Object *ob);
	void build_object_transform(Scene *scene, Object *ob);
	void build_object_constraints(Scene *scene, Object *ob);
	void build_pose_constraints(Scene *scene, Object *ob, bPoseChannel *pchan);
	void build_rigidbody(Scene *scene);
	void build_particles(Scene *scene, Object *ob);
	void build_particle_settings(ParticleSettings *part);
	void build_cloth(Scene *scene, Object *object);
	void build_animdata(ID *id);
	OperationDepsNode *build_driver(ID *id, FCurve *fcurve);
	void build_ik_pose(Scene *scene,
	                   Object *ob,
	                   bPoseChannel *pchan,
	                   bConstraint *con);
	void build_splineik_pose(Scene *scene,
	                         Object *ob,
	                         bPoseChannel *pchan,
	                         bConstraint *con);
	void build_rig(Scene *scene, Object *ob);
	void build_proxy_rig(Object *ob);
	void build_shapekeys(Key *key);
	void build_obdata_geom(Scene *scene, Object *ob);
	void build_camera(Object *ob);
	void build_lamp(Object *ob);
	void build_nodetree(bNodeTree *ntree);
	void build_material(Material *ma);
	void build_texture(Tex *tex);
	void build_texture_stack(MTex **texture_stack);
	void build_image(Image *image);
	void build_world(World *world);
	void build_compositor(Scene *scene);
	void build_gpencil(bGPdata *gpd);
	void build_cachefile(CacheFile *cache_file);
	void build_mask(Mask *mask);
	void build_movieclip(MovieClip *clip);
	void build_lightprobe(Object *object);

	struct LayerCollectionState {
		int index;
		LayerCollection *parent;
	};
	void build_layer_collection(Scene *scene,
	                            LayerCollection *layer_collection,
	                            LayerCollectionState *state);
	void build_layer_collections(Scene *scene,
	                             ListBase *layer_collections,
	                             LayerCollectionState *state);
	void build_scene_layer_collections(Scene *scene);
protected:
	Main *m_bmain;
	Depsgraph *m_graph;
	GHash *m_cow_id_hash;
};

}  // namespace DEG
