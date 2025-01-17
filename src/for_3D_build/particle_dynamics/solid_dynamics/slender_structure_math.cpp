#include "slender_structure_math.h"

namespace SPH
{
	//=====================================================================================================//
	namespace slender_structure_dynamics
	{
		//=================================================================================================//
	
		Vec3d getVectorAfterThinStructureRotation(const Vec3d &initial_vector, const Vec3d &rotation_angles)
		{
                      /*  Real sin_angle_x = sin(rotation_angles[0]);
                        Real cos_angle_x = cos(rotation_angles[0]);

                        Real sin_angle_y = sin(rotation_angles[1]);
                        Real cos_angle_y = cos(rotation_angles[1]);

                        Real sin_angle_z = sin(rotation_angles[2]);
                        Real cos_angle_z = cos(rotation_angles[2]);*/

                        Real theta = sqrt(rotation_angles[0] * rotation_angles[0] + rotation_angles[1] * rotation_angles[1] + rotation_angles[2] * rotation_angles[2]);
                        Mat3d R1 = Mat3d::Zero();
                        R1(0, 1) = -rotation_angles[2];
                        R1(0, 2) = rotation_angles[1];
                        R1(1, 0) = rotation_angles[2];
                        R1(1, 2) = -rotation_angles[0];
                        R1(2, 0) = -rotation_angles[1];
                        R1(2, 1) = rotation_angles[0];

                        Mat3d rotation_matrix = Mat3d::Identity() + sin(theta) / (theta + Eps) * R1 + (1 - cos(theta)) / (theta * theta + Eps) * R1 * R1;

                        return rotation_matrix * initial_vector;
		}
	
		//=================================================================================================//
		Vec3d getVectorChangeRateAfterThinStructureRotation(const Vec3d &initial_vector, const Vec3d &rotation_angles, const Vec3d &angular_vel)
		{
			Real sin_rotation_0 = sin(rotation_angles[0]);
			Real cos_rotation_0 = cos(rotation_angles[0]);

			Real sin_rotation_1 = sin(rotation_angles[1]);
			Real cos_rotation_1 = cos(rotation_angles[1]);

			Real dpseudo_n_dt_0 = -sin_rotation_0 * sin_rotation_1 * angular_vel[0] + cos_rotation_0 * cos_rotation_1 * angular_vel[1];
			Real dpseudo_n_dt_1 = -cos_rotation_0 * angular_vel[0];
			Real dpseudo_n_dt_2 = -sin_rotation_0 * cos_rotation_1 * angular_vel[0] - cos_rotation_0 * sin_rotation_1 * angular_vel[1];

			return Vec3d(dpseudo_n_dt_0, dpseudo_n_dt_1, dpseudo_n_dt_2);
		}
		//=================================================================================================//
	
		Vec3d getRotationFromPseudoNormalForFiniteDeformation(const Vec3d &dpseudo_n_d2t, const Vec3d &rotation, const Vec3d &angular_vel, Real dt)
		{
			Real sin_rotation_0 = sin(rotation[0]);
			Real cos_rotation_0 = cos(rotation[0]);
			Real sin_rotation_1 = sin(rotation[1]);
			Real cos_rotation_1 = cos(rotation[1]);

			Real rotation_0_a = -(dpseudo_n_d2t[2] * cos_rotation_1 + dpseudo_n_d2t[0] * sin_rotation_1
								  + angular_vel[1] * angular_vel[1] * cos_rotation_0
								  + angular_vel[0] * angular_vel[0] * cos_rotation_0);
			Real rotation_0_b = sin_rotation_0 * angular_vel[0] * angular_vel[0] - dpseudo_n_d2t[1];
			Real angle_vel_dt_0 = sin_rotation_0 * rotation_0_a + cos_rotation_0 * rotation_0_b;

			Real rotation_1_a = dpseudo_n_d2t[0] * cos_rotation_1 - dpseudo_n_d2t[2] * sin_rotation_1
								+ 2.0 * angular_vel[1] * angular_vel[0] * sin_rotation_0;
			Real rotation_1_b1 = dpseudo_n_d2t[0] * cos_rotation_0
								 + angular_vel[1] * angular_vel[1] * cos_rotation_0 * cos_rotation_0 * sin_rotation_1
								 + angular_vel[0] * angular_vel[0] * sin_rotation_1
								 - dpseudo_n_d2t[1] * sin_rotation_1 * sin_rotation_0
								 + 2.0 * angular_vel[1] * angular_vel[0] * cos_rotation_1 * cos_rotation_0 * sin_rotation_0;
			Real rotation_1_b2 = -(dpseudo_n_d2t[2] * cos_rotation_0
								   + angular_vel[1] * angular_vel[1] * cos_rotation_1 * cos_rotation_0 * cos_rotation_0
								   + angular_vel[0] * angular_vel[0] * cos_rotation_1
								   - dpseudo_n_d2t[1] * cos_rotation_1 * sin_rotation_0
								   - 2.0 * angular_vel[1] * angular_vel[0] * cos_rotation_0 * sin_rotation_1 * sin_rotation_0);
			Real angle_vel_dt_1 = rotation_1_a * rotation_1_a * (rotation_1_b1 * cos_rotation_1 + rotation_1_b2 * sin_rotation_1)
								  / (rotation_1_b1 * rotation_1_b1 + rotation_1_b2 * rotation_1_b2 + Eps);
					
			return Vec3d(angle_vel_dt_0, angle_vel_dt_1, 0.0);
		}
		//=================================================================================================//
		
        Vec3d getRotationFromPseudoNormalForSmallDeformation(const Vec3d &dpseudo_b_n_d2t, const Vec3d &dpseudo_n_d2t, const Vec3d &rotation, const Vec3d &angular_vel, Real dt)
		{
            return Vec3d(0.0, dpseudo_n_d2t[0], 0.0);
		}
		//=================================================================================================//

		Vec3d getRotationFromPseudoNormalForSmallDeformation_b(const Vec3d &dpseudo_b_n_d2t, const Vec3d &dpseudo_n_d2t, const Vec3d &rotation, const Vec3d &angular_vel, Real dt)
        {
            return Vec3d(0.0, 0.0, dpseudo_b_n_d2t[0]);
        }
                //=================================================================================================//

	
		Vec3d getNormalFromDeformationGradientTensor(const Mat3d &F)
		{
			return F.col(0).cross(F.col(1)).normalized();
		}
        Vec3d getBinormalFromDeformationGradientTensor(const Mat3d &F)
        {
            return (F.col(2).cross(F.col(0))).normalized();
        }
		//=================================================================================================//
		Vecd getLinearVariableJump(const Vecd &e_ij, const Real &r_ij, const Vecd &particle_i_value,
			const Matd &gradient_particle_i_value, const Vecd &particle_j_value, const Matd &gradient_particle_j_value)
		{
			return particle_i_value - particle_j_value
				   - 0.5 * r_ij * (gradient_particle_i_value + gradient_particle_j_value) * e_ij;
		}
		//=================================================================================================//
		Vecd getWENOVariableJump(const Vecd &e_ij, const Real &r_ij, const Vecd &particle_i_value,
			const Matd &gradient_particle_i_value, const Vecd &particle_j_value, const Matd &gradient_particle_j_value)
		{
			return getWENOLeftState(e_ij, r_ij, particle_i_value,
									gradient_particle_i_value, particle_j_value, gradient_particle_j_value)
				   - getWENORightState(e_ij, r_ij, particle_i_value,
									   gradient_particle_i_value, particle_j_value, gradient_particle_j_value);
		}
		//=================================================================================================//
		Vecd getWENOStateWithStencilPoints(const Vecd &v1, const Vecd &v2, const Vecd &v3, const Vecd &v4)
		{
			Vecd f1 = 0.5 * v2 + 0.5 * v3;
			Vecd f2 = -0.5 * v1 + 1.5 * v2;
			Vecd f3 = v2 / 3.0 + 5.0 * v3 / 6.0 - v4 / 6.0;

			Real epsilon = 1.0e-6;
			Real s1 = (v2 - v3).dot(v2 - v3) + epsilon;
			Real s2 = (v2 - v1).dot(v2 - v1) + epsilon;
			Real s3 = (3.0 * v2 - 4.0 * v3 + v4).dot(3.0 * v2 - 4.0 * v3 + v4) / 4.0
					  + 13.0 * (v2 - 2.0 * v3 + v4).dot(v2 - 2.0 * v3 + v4) / 12.0 + epsilon;
			Real s12 = 13.0 * (v1 - 2.0 * v2 + v3).dot(v1 - 2.0 * v2 + v3) / 12.0
					   + (v1 - v3).dot(v1 - v3) / 4.0 + epsilon;
			Real s4 = (v1.dot(6649.0 * v1 - 30414.0 * v2 + 23094.0 * v3 - 5978.0 * v4)
					   + 3.0 * v2.dot(13667.0 * v2 - 23534.0 * v3 + 6338.0 * v4)
					   + 3.0 * v3.dot(11147.0 * v3 - 6458.0 * v4)
					   + 3169.0 * v4.dot(v4)) / 2880.0;
			Real tau_4 = s4 - 0.5 * (s1 + s2);

			Real alpha_1 = (1.0 + (tau_4 / s1) * (tau_4 / s12)) / 3.0;
			Real alpha_2 = (1.0 + (tau_4 / s2) * (tau_4 / s12)) / 6.0;
			Real alpha_3 = (1.0 + tau_4 / s3) / 2.0;
			Real w_1 = alpha_1 / (alpha_1 + alpha_2 + alpha_3);
			Real w_2 = alpha_2 / (alpha_1 + alpha_2 + alpha_3);
			Real w_3 = alpha_3 / (alpha_1 + alpha_2 + alpha_3);

			return w_1 * f1 + w_2 * f2 + w_3 * f3;
		}
		//=================================================================================================//
		Vecd getWENOLeftState(const Vecd &e_ij, const Real &r_ij, const Vecd &particle_i_value,
			const Matd &gradient_particle_i_value, const Vecd &particle_j_value, const Matd &gradient_particle_j_value)
		{
			Vecd v1 = particle_i_value + gradient_particle_i_value * e_ij * r_ij;
			Vecd v2 = particle_i_value;
			Vecd v3 = particle_j_value;
			Vecd v4 = particle_j_value - gradient_particle_j_value * e_ij * r_ij;

			return getWENOStateWithStencilPoints(v1, v2, v3, v4);
		}
		//=================================================================================================//
		Vecd getWENORightState(const Vecd &e_ij, const Real &r_ij, const Vecd &particle_i_value,
			const Matd &gradient_particle_i_value, const Vecd &particle_j_value, const Matd &gradient_particle_j_value)
		{
			Vecd v1 = particle_j_value - gradient_particle_j_value * e_ij * r_ij;
			Vecd v2 = particle_j_value;
			Vecd v3 = particle_i_value;
			Vecd v4 = particle_i_value + gradient_particle_i_value * e_ij * r_ij;

			return getWENOStateWithStencilPoints(v1, v2, v3, v4);
		}
		//=================================================================================================//
		
		Vec3d getRotationJump(const Vec3d &pseudo_n_jump, const Mat3d &transformation_matrix)
		{
			Vec3d local_rotation_jump = Vec3d::Zero();
			Vec3d local_pseuodo_n_jump = transformation_matrix * pseudo_n_jump;
			local_rotation_jump[0] = local_pseuodo_n_jump[0];
			local_rotation_jump[1] = local_pseuodo_n_jump[1];
			return transformation_matrix.transpose() * local_rotation_jump;
		}
		//=================================================================================================//
	
		Mat3d getCorrectedAlmansiStrain(const Mat3d &current_local_almansi_strain, const Real &nu_)
		{
			Mat3d corrected_almansi_strain = current_local_almansi_strain;
			corrected_almansi_strain(2,2)
				= -nu_ * (current_local_almansi_strain(0,0) + current_local_almansi_strain(1,1)) / (1.0 - nu_);
			return corrected_almansi_strain;
		}
		//=================================================================================================//

		Mat3d getCorrectionMatrix(const Mat3d &local_deformation_part_one)
		{
			Mat3d correction_matrix = Mat3d::Zero();
			correction_matrix.block<2,2>(0,0) = local_deformation_part_one.block<2,2>(0,0).inverse();
			return correction_matrix;
		}

		Mat3d getCorrectionMatrix_beam(const Mat3d &local_deformation_part_one)
         {
            Mat3d correction_matrix = Mat3d::Zero();
            correction_matrix.block<1, 1>(0, 0) = local_deformation_part_one.block<1, 1>(0, 0).inverse();
            return correction_matrix;
         }
		//=================================================================================================//
	}
}
