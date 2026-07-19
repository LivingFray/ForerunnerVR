#include "animation_graph_definitions.h"

#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/math/matrix_math.h"
#include "payload/delta/blam/tag_files/tag_groups.h"

void c_animation2_rig_definition__node_matrices_from_orientations::Patch(c_animation2_rig_definition* This, real_matrix4x3* node_matrices, real_orientation* node_orientations, real_matrix4x3* root)
{
	if (This->nodes.count <= 0)
	{
		FORERUNNER_LOG(Delta_AnimGraph, "Invalid nodes count: {}", This->nodes.count);
		return;
	}

	int queue[256] = {};
	int read_idx = 0;
	int write_idx = 1;
	queue[0] = 0;
	const c_animation2_rig_node* node_list = (c_animation2_rig_node*)tag_block_get_element(This->nodes);

	while (read_idx != write_idx)
	{
		const int32_t queue_ptr = queue[read_idx++];

		const c_animation2_rig_node& node = node_list[queue_ptr];

		real_matrix4x3* parent_matrix = root;
		if (queue_ptr != 0)
		{
			parent_matrix = &node_matrices[node.parent_node_index];
		}

		real_matrix4x3 local_transform;
		matrix4x3_from_orientation(&local_transform, &node_orientations[queue_ptr]);
		matrix4x3_multiply(parent_matrix, &local_transform, &node_matrices[queue_ptr]);

		if (node.next_sibling_node_index != -1)
		{
			queue[write_idx] = node.next_sibling_node_index;
			write_idx++;
		}
		if (node.first_child_node_index != -1)
		{
			queue[write_idx] = node.first_child_node_index;
			write_idx++;
		}
	}
}