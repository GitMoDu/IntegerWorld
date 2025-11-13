#ifndef _INTEGER_WORLD_ASSETS_CAMERAS_h
#define _INTEGER_WORLD_ASSETS_CAMERAS_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include <IntegerWorld.h>

namespace IntegerWorld
{
	namespace Cameras
	{
		template<const uint32_t PeriodMillis = 10>
		class FreeCameraTask : private TS::Task
		{
		private:
			static constexpr uint8_t LookGainShifts = 5;
			static constexpr uint8_t MoveGainShifts = 11;

		public:
			ufraction16_t LookXSensitivity = UFraction16::GetScalar<int32_t>(150, 1000);
			ufraction16_t LookYSensitivity = UFraction16::GetScalar<int32_t>(150, 1000);

			ufraction16_t MoveXYSensitivity = UFraction16::GetScalar<int32_t>(150, 1000);
			ufraction16_t MoveZSensitivity = UFraction16::GetScalar<int32_t>(100, 1000);

		private:
			Filters::LowPassI16<11> MoveZFilter{};
			Filters::LowPassI16<11> MoveXFilter{};
			Filters::LowPassI16<11> MoveYFilter{};

			Filters::EmaI16<3> LookXFilter{};
			Filters::EmaI16<3> LookYFilter{};

			vertex16_t Position{};
			int16_t Yaw = 0;
			int32_t Pitch = 0;
			camera_state_t* CameraControls;

		public:
			int16_t Roll = 0;

		private:
			uint32_t LastUpdate = 0;

		public:
			FreeCameraTask(TS::Scheduler& scheduler,
				camera_state_t* cameraControls)
				: TS::Task(PeriodMillis, TASK_FOREVER, &scheduler, true)
				, CameraControls(cameraControls)
			{
			}

			bool Callback() final
			{
				const uint32_t timestamp = micros();
				const int32_t deltaTime = timestamp - LastUpdate;
				const int32_t deltaScale = deltaTime >> 8;

				LastUpdate = timestamp;

				const uint8_t filterSteps = 1 + ((deltaTime / PeriodMillis) / 1000);
				for (uint8_t i = 0; i < filterSteps; i++)
				{
					MoveZFilter.Step();
					MoveXFilter.Step();
					MoveYFilter.Step();
					LookXFilter.Step();
					LookYFilter.Step();
				}

				const int16_t lookX = -Fraction(LookXSensitivity, LookXFilter.Get());

				Yaw += (deltaScale * lookX) >> LookGainShifts;
				Yaw %= ANGLE_RANGE;

				const fraction16_t lookY = -Fraction16::GetScalar<int16_t>(LookYFilter.Get(), INT16_MAX);
				const int16_t lookYScaled = Fraction(Fraction(LookYSensitivity, lookY), ANGLE_180);
				Pitch += (deltaScale * lookYScaled) >> LookGainShifts;

				// Saturate Pitch to [-ANGLE_90, +ANGLE_90]
				if (Pitch < -int32_t(ANGLE_90))
				{
					Pitch = -int32_t(ANGLE_90);
				}
				else if (Pitch > int32_t(ANGLE_90))
				{
					Pitch = int32_t(ANGLE_90);
				}

				// --- Integer movement ---
				const int16_t moveForward = Fraction(MoveXYSensitivity, MoveYFilter.Get());
				const int16_t moveStrafe = -Fraction(MoveXYSensitivity, MoveXFilter.Get());
				const int16_t moveUp = -Fraction(MoveZSensitivity, MoveZFilter.Get());

				// 0º yaw -> +Z
				const fraction16_t sinYaw = Sine16(Yaw);
				const fraction16_t cosYaw = Cosine16(Yaw);

				// Move camera in world
				Position.x += (deltaScale * (Fraction(sinYaw, moveForward) + Fraction(cosYaw, moveStrafe))) >> MoveGainShifts;
				Position.y += (deltaScale * moveUp) >> MoveGainShifts;
				Position.z += (deltaScale * (Fraction(cosYaw, moveForward) + Fraction(fraction16_t(-sinYaw), moveStrafe))) >> MoveGainShifts;

				if (CameraControls != nullptr)
				{
					CameraControls->Rotation.y = Yaw;
					const angle_t pitchAngle =
						(Pitch >= 0) ? angle_t(Pitch) : angle_t(int32_t(ANGLE_RANGE) + Pitch);
					CameraControls->Rotation.x = pitchAngle;
					CameraControls->Rotation.z = Roll;
					CameraControls->Position = Position;
				}

				return true;
			}

			void Set(const int16_t moveX, const int16_t moveY, const int16_t moveZ, const int16_t lookX, const int16_t lookY)
			{
				MoveXFilter.Set(CurveMove(moveX));
				MoveYFilter.Set(CurveMove(moveY));
				MoveZFilter.Set(CurveMove(moveZ));

				LookXFilter.Set(CurveLook(lookX));
				LookYFilter.Set(CurveLook(lookY));
			}

			void ResetFilters()
			{
				MoveZFilter.Clear();
				MoveXFilter.Clear();
				MoveYFilter.Clear();
				LookXFilter.Clear();
				LookYFilter.Clear();
			}

			void ResetPosition()
			{
				Position.x = 0;
				Position.y = 0;
				Position.z = 0;

				MoveZFilter.Clear();
				MoveXFilter.Clear();
				MoveYFilter.Clear();
			}

			void ResetCamera()
			{
				Yaw = 0;
				Pitch = 0;
				Roll = 0;

				LookXFilter.Clear();
				LookYFilter.Clear();

				LastUpdate = micros();
			}

		private:
			static int16_t CurveLook(const int16_t in)
			{
				return CurvePower(CurvePower(in));
			}

			static int16_t CurveMove(const int16_t in)
			{
				return CurvePower(in);
			}

			static int16_t CurvePower(const int16_t in)
			{
				const int8_t sign = (in < 0) ? -1 : 1;
				return static_cast<int16_t>(SignedRightShift<int32_t, 16>(static_cast<int32_t>(in) * in)) * sign;
			}
		};
	}
}

#endif