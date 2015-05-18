/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <array>
#include <cmath>

namespace gls {

////////////////////////////////////////////////////////////////////////////////
/// \brief Utility class representing a 3D camera
///
/// \tparam T The underlying type of the camera's parameters (float, double, etc.)
///
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class camera {
public:
	typedef T value_type; ///< The underlying type of the camera's parameters

	typedef std::array<value_type, 3> vector_type; ///< Camera vector type
	typedef std::array<value_type, 4> quaternion_type; ///< Camera quaternion type
	typedef std::array<value_type, 16> matrix_type; ///< Camera matrix type

	constexpr static value_type pi = static_cast<value_type>( 3.14159265359 ); ///< Value of pi used by the camera

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor
	///
	//////////////////////////////////////////////////////////////////////////////
	camera() = default;

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor from projection parameters
	///
	/// This constructor constructs the camera from the given projection
	/// parameters. The camera is positioned at (0, 0, 0), with up vector
	/// (0, 1, 0) and facing down the negative z-axis (0, 0, -1).
	///
	/// \param fov Field of view of the camera in radians
	/// \param aspect Aspect ratio (width / height) of the camera
	/// \param near_distance Distance to the near plane
	/// \param far_distance Distance to the far plane
	///
	//////////////////////////////////////////////////////////////////////////////
	camera( value_type fov, value_type aspect, value_type near_distance, value_type far_distance ) :
		m_fov( fov ),
		m_aspect( aspect ),
		m_near( near_distance ),
		m_far( far_distance )
	{

	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Constructor from projection and view parameters
	///
	/// This constructor constructs the camera from the given projection and view
	/// parameters.
	///
	/// \param fov Field of view of the camera in radians
	/// \param aspect Aspect ratio (width / height) of the camera
	/// \param near_distance Distance to the near plane
	/// \param far_distance Distance to the far plane
	/// \param position Position of the camera
	/// \param direction Direction the camera is facing in
	/// \param up Up vector of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	camera( value_type fov, value_type aspect, value_type near_distance, value_type far_distance, vector_type position, vector_type direction, vector_type up ) :
		m_position( position ),
		m_fov( fov ),
		m_aspect( aspect ),
		m_near( near_distance ),
		m_far( far_distance )
	{
		m_direction = vector_normalize( direction );
		m_up = vector_normalize( up );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the field of view of the camera
	///
	/// \return Field of view of the camera in radians
	///
	//////////////////////////////////////////////////////////////////////////////
	value_type get_fov() const {
		return m_fov;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the field of view of the camera
	///
	/// \param fov Field of view of the camera in radians
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_fov( value_type fov ) {
		if( m_fov == fov ) {
			return;
		}

		m_fov = fov;
		m_projection_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the aspect ratio of the camera
	///
	/// \return Aspect ratio of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	value_type get_aspect() const {
		return m_aspect;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the aspect ratio of the camera
	///
	/// \param aspect Aspect ratio of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_aspect( value_type aspect ) {
		if( m_aspect == aspect ) {
			return;
		}

		m_aspect = aspect;
		m_projection_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the distance to the near plane of the camera
	///
	/// \return Distance to the near plane of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	value_type get_near_distance() const {
		return m_near;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the distance to the near plane of the camera
	///
	/// \param near_distance Distance to the near plane of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_near_distance( value_type near_distance ) {
		if( m_near == near_distance ) {
			return;
		}

		m_near = near_distance;
		m_projection_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the distance to the far plane of the camera
	///
	/// \return Distance to the far plane of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	value_type get_far_distance() const {
		return m_far;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the distance to the far plane of the camera
	///
	/// \param far_distance Distance to the far plane of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_far_distance( value_type far_distance ) {
		if( m_far == far_distance ) {
			return;
		}

		m_far = far_distance;
		m_projection_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the position of the camera
	///
	/// \return Position of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	const vector_type& get_position() const {
		return m_position;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the position of the camera
	///
	/// \param position Position of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_position( const vector_type& position ) {
		if( m_position == position ) {
			return;
		}

		m_position = position;
		m_view_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the direction the camera is facing in
	///
	/// \return Direction the camera is facing in
	///
	//////////////////////////////////////////////////////////////////////////////
	const vector_type& get_direction() const {
		return m_direction;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the direction the camera is facing in
	///
	/// \param direction Direction the camera is facing in
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_direction( const vector_type& direction ) {
		const auto new_direction = vector_normalize( direction );

		if( m_direction == new_direction ) {
			return;
		}

		m_direction = new_direction;
		m_view_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the up vector of the camera
	///
	/// \return Up vector of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	const vector_type& get_up() const {
		return m_up;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the up vector of the camera
	///
	/// \param up Up vector of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_up( const vector_type& up ) {
		const auto new_up = vector_normalize( up );

		if( m_up == new_up ) {
			return;
		}

		m_up = new_up;
		m_view_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move the camera in world space
	///
	/// Moves the camera by the given offset in world space i.e. the global
	/// coordinate system.
	///
	/// \param offset Offset to move the camera by
	///
	//////////////////////////////////////////////////////////////////////////////
	void move( vector_type offset ) {
		set_position( { { m_position[0] + offset[0], m_position[1] + offset[1], m_position[2] + offset[2] } } );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Move the camera in camera space
	///
	/// Moves the camera by the given offset in camera space i.e. the local
	/// coordinate system.
	///
	/// \param offset Offset to move the camera by
	///
	//////////////////////////////////////////////////////////////////////////////
	void move_relative( vector_type offset ) {
		const auto right_vector = vector_normalize( vector_cross( m_direction, m_up ) );

		vector_type delta{ { -m_direction[0] * offset[2], -m_direction[1] * offset[2], -m_direction[2] * offset[2] } };

		delta[0] += m_up[0] * offset[1] - right_vector[0] * offset[0];
		delta[1] += m_up[1] * offset[1] - right_vector[1] * offset[0];
		delta[2] += m_up[2] * offset[1] - right_vector[2] * offset[0];

		move( delta );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Rotate the camera in world space
	///
	/// \param rotation Rotation quaternion
	///
	//////////////////////////////////////////////////////////////////////////////
	void rotate( quaternion_type rotation ) {
		auto direction = quaternion_type{ m_direction[0], m_direction[1], m_direction[2], 0 };
		direction = quaternion_multiply( quaternion_multiply( rotation, direction ), quaternion_conjugate( rotation ) );
		m_direction = vector_normalize( { direction[0], direction[1], direction[2] } );

		auto up = quaternion_type{ m_up[0], m_up[1], m_up[2], 0 };
		up = quaternion_multiply( quaternion_multiply( rotation, up ), quaternion_conjugate( rotation ) );
		m_up = vector_normalize( { up[0], up[1], up[2] } );

		m_view_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Rotate the camera in world space
	///
	/// \param axis Axis of rotation
	/// \param angle Angle to rotate by in radians
	///
	//////////////////////////////////////////////////////////////////////////////
	void rotate( vector_type axis, value_type angle ) {
		quaternion_type rotation{
			axis[0] * std::sin( angle / 2 ),
			axis[1] * std::sin( angle / 2 ),
			axis[2] * std::sin( angle / 2 ),
			std::cos( angle / 2 )
		};

		rotate( rotation );
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Set the orientation of the camera from Tait-Bryan angles
	///
	/// The rotations are performed in the following order: yaw-pitch-roll
	///
	/// \param yaw Yaw angle in radians
	/// \param pitch Pitch angle in radians
	/// \param roll Roll angle in radians
	///
	//////////////////////////////////////////////////////////////////////////////
	void set_orientation_taitbryan( value_type yaw, value_type pitch, value_type roll ) {
		auto direction = quaternion_type{ { 0, 0, -1, 0 } };

		{
			quaternion_type rotation{ {
					0,
					std::sin( -yaw / 2 ),
					0,
					std::cos( -yaw / 2 )
			} };

			direction = quaternion_multiply( quaternion_multiply( rotation, direction ), quaternion_conjugate( rotation ) );
			direction = quaternion_normalize( direction );
		}

		const auto right = vector_normalize( vector_cross( { { 0, 1, 0 } }, { { direction[0], direction[1], direction[2] } } ) );

		{
			quaternion_type rotation{ {
					right[0] * std::sin( pitch / 2 ),
					right[1] * std::sin( pitch / 2 ),
					right[2] * std::sin( pitch / 2 ),
					std::cos( pitch / 2 )
			} };

			direction = quaternion_multiply( quaternion_multiply( rotation, direction ), quaternion_conjugate( rotation ) );
		}

		m_direction = vector_normalize( { { direction[0], direction[1], direction[2] } } );

		const auto up_vector = vector_normalize( vector_cross( m_direction, right ) );
		auto up_quaternion = quaternion_type{ { up_vector[0], up_vector[1], up_vector[2], 0 } };

		{
			quaternion_type rotation{ {
					m_direction[0] * std::sin( roll / 2 ),
					m_direction[1] * std::sin( roll / 2 ),
					m_direction[2] * std::sin( roll / 2 ),
					std::cos( roll / 2 )
			} };

			up_quaternion = quaternion_multiply( quaternion_multiply( rotation, up_quaternion ), quaternion_conjugate( rotation ) );
		}

		m_up = vector_normalize( { { up_quaternion[0], up_quaternion[1], up_quaternion[2] } } );

		m_view_dirty = true;
		m_matrix_dirty = true;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the view matrix of the camera
	///
	/// \return View matrix of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	const matrix_type& get_view() const {
		update();

		return m_view;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the projection matrix of the camera
	///
	/// \return Projection matrix of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	const matrix_type& get_projection() const {
		update();

		return m_projection;
	}

	//////////////////////////////////////////////////////////////////////////////
	/// \brief Get the pre-multiplied projection and view matrix of the camera
	///
	/// get_matrix() = get_projection() * get_view()
	///
	/// \return Pre-multiplied projection and view matrix of the camera
	///
	//////////////////////////////////////////////////////////////////////////////
	const matrix_type& get_matrix() const {
		update();

		return m_matrix;
	}

private:
	static value_type vector_length( const vector_type& v ) {
		return std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
	}

	static vector_type vector_normalize( const vector_type& v ) {
		auto length = vector_length( v );
		return vector_type{ { v[0] / length, v[1] / length, v[2] / length } };
	}

	static value_type vector_dot( const vector_type& v, const vector_type& w ) {
		return v[0] * w[0] + v[1] * w[1] + v[2] * w[2];
	}

	static vector_type vector_cross( const vector_type& v, const vector_type& w ) {
		return { {
				v[1] * w[2] - v[2] * w[1],
				v[2] * w[0] - v[0] * w[2],
				v[0] * w[1] - v[1] * w[0]
		} };
	}

	static value_type quaternion_length( const quaternion_type& q ) {
		return std::sqrt( q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3] );
	}

	static quaternion_type quaternion_normalize( const quaternion_type& q ) {
		auto length = quaternion_length( q );
		return quaternion_type{ { q[0] / length, q[1] / length, q[2] / length, q[3] / length } };
	}

	static quaternion_type quaternion_conjugate( const quaternion_type& q ) {
		return quaternion_type{ { -q[0], -q[1], -q[2], q[3] } };
	}

	static quaternion_type quaternion_multiply( const quaternion_type& q, const quaternion_type& r ) {
		return quaternion_type{ {
				q[0] * r[3] + q[3] * r[0] + q[1] * r[2] - q[2] * r[1],
				q[1] * r[3] + q[3] * r[1] + q[2] * r[0] - q[0] * r[2],
				q[2] * r[3] + q[3] * r[2] + q[0] * r[1] - q[1] * r[0],
				q[3] * r[3] - q[0] * r[0] - q[1] * r[1] - q[2] * r[2]
		} };
	}

	static matrix_type matrix_mult( const matrix_type& m, const matrix_type& n ) {
		return { {
				m[0] * n[0] + m[4] * n[1] + m[8] * n[2] + m[12] * n[3],
				m[1] * n[0] + m[5] * n[1] + m[9] * n[2] + m[13] * n[3],
				m[2] * n[0] + m[6] * n[1] + m[10] * n[2] + m[14] * n[3],
				m[3] * n[0] + m[7] * n[1] + m[11] * n[2] + m[15] * n[3],
				m[0] * n[4] + m[4] * n[5] + m[8] * n[6] + m[12] * n[7],
				m[1] * n[4] + m[5] * n[5] + m[9] * n[6] + m[13] * n[7],
				m[2] * n[4] + m[6] * n[5] + m[10] * n[6] + m[14] * n[7],
				m[3] * n[4] + m[7] * n[5] + m[11] * n[6] + m[15] * n[7],
				m[0] * n[8] + m[4] * n[9] + m[8] * n[10] + m[12] * n[11],
				m[1] * n[8] + m[5] * n[9] + m[9] * n[10] + m[13] * n[11],
				m[2] * n[8] + m[6] * n[9] + m[10] * n[10] + m[14] * n[11],
				m[3] * n[8] + m[7] * n[9] + m[11] * n[10] + m[15] * n[11],
				m[0] * n[12] + m[4] * n[13] + m[8] * n[14] + m[12] * n[15],
				m[1] * n[12] + m[5] * n[13] + m[9] * n[14] + m[13] * n[15],
				m[2] * n[12] + m[6] * n[13] + m[10] * n[14] + m[14] * n[15],
				m[3] * n[12] + m[7] * n[13] + m[11] * n[14] + m[15] * n[15]
		} };
	}

	void update() const {
		// Update projection matrix
		if( m_projection_dirty ) {
			m_projection = { {
					1 / ( m_aspect * std::tan( m_fov / 2 ) ), 0, 0, 0,
					0, 1 / std::tan( m_fov / 2 ), 0, 0,
					0, 0, -( m_far + m_near ) / ( m_far - m_near ), -1,
					0, 0, -( 2 * m_far * m_near ) / ( m_far - m_near ), 0
			} };

			m_projection_dirty = false;
		}

		// Update view matrix
		if( m_view_dirty ) {
			const auto d = m_direction;
			const auto s = vector_normalize( vector_cross( m_direction, m_up ) );
			const auto u = vector_cross( s, d );

			m_view = { {
					s[0], u[0], -d[0], 0,
					s[1], u[1], -d[1], 0,
					s[2], u[2], -d[2], 0,
					0, 0, 0, 1
			} };

			m_view = matrix_mult( m_view, { {
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					-m_position[0], -m_position[1], -m_position[2], 1
			} } );

			m_view_dirty = false;
		}

		// Update complete matrix
		if( m_matrix_dirty ) {
			m_matrix = matrix_mult( m_projection, m_view );

			m_matrix_dirty = false;
		}
	}

	mutable matrix_type m_projection;
	mutable matrix_type m_view;
	mutable matrix_type m_matrix;

	vector_type m_position = { { 0, 0, 0 } };
	vector_type m_direction = { { 0, 0, -1 } };
	vector_type m_up = { { 0, 1, 0 } };

	value_type m_fov = pi * 90 / 180;
	value_type m_aspect = 1;
	value_type m_near = 1;
	value_type m_far = 1000;

	mutable bool m_projection_dirty = true;
	mutable bool m_view_dirty = true;
	mutable bool m_matrix_dirty = true;
};

}

////////////////////////////////////////////////////////////////////////////////
/// \class gls::camera
/// \ingroup utilities
///
/// A gls::camera is a lightweight utility class that represents a 3D camera.
///
/// gls::camera stores both projection parameters such as field of view, aspect
/// ratio, near plane distance and far plane distance and view parameters such
/// as camera position, direction and up vector.
///
/// By default, a camera is constructed with a field of view of 0.5pi, an aspect
/// ratio of 1:1, a near plane distance of 1 and a far plane distance of 1000.
/// It is positioned at the world origin and faces down the negative z-axis with
/// the up vector pointing down the positive y-axis.
///
/// A camera can be moved or rotated.
///
/// Movement is done either relative to the world coordinate system or the
/// camera coordinate system with move() and move_relative() respectively. In
/// the former case, the camera's position is simply offset by the passed value.
/// In the latter case, the camera moves according to its local coordinate
/// system "right", "up" or "forward" from the perspective of the camera itself.
///
/// Rotation can only be done relative to the world coordinate system.
/// Consecutive rotations are applied one after another and accumulated into the
/// orientation of the camera. Setting the orientation via Tait-Bryan angles is
/// also possible.
///
/// When done setting the parameters of the camera, the projection and view
/// matrices can be queried and sent to the GL either individually, using
/// get_view() and get_projection() or pre-multiplied using get_matrix().
///
/// Example usage:
/// \code
/// auto camera = gls::camera<float>( gls::camera<float>::pi * .5f, 1.f, .1f, 100.f );
/// camera.set_position( { 1.f, 2.f, 3.f } );
/// camera.set_orientation_taitbryan( { gls::camera<float>::pi / 4.f, 0.f, 0.f } );
/// ...
/// camera.move_relative( { 0.f, 0.f, -10.f } );
/// ...
/// program.uniform_matrix4( "view_projection_matrix", 1, GL_FALSE, camera.get_matrix().data() );
/// \endcode
///
////////////////////////////////////////////////////////////////////////////////
