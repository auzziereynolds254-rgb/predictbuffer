#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
using namespace geode::prelude;

class $modify(PBPlayer, PlayerObject) {
	struct Fields {
		bool pending = false;
		bool releaseQueued = false;
		bool replaying = false;
		int framesLeft = 0;
	};

	bool isCubeMode() {
		return !(m_isShip || m_isBird || m_isBall || m_isDart ||
		         m_isRobot || m_isSpider || m_isSwing);
	}

	bool isFalling() {
		return m_isUpsideDown ? m_yVelocity > 0.0 : m_yVelocity < 0.0;
	}

	bool pushButton(PlayerButton btn) {
		auto f = m_fields.self();
		if (f->replaying || btn != PlayerButton::Jump ||
		    !Mod::get()->getSettingValue<bool>("enabled"))
			return PlayerObject::pushButton(btn);

		if (isCubeMode() && !m_isOnGround && isFalling()) {
			f->pending = true;
			f->releaseQueued = false;
			f->framesLeft = Mod::get()->getSettingValue<int64_t>("window");
			return true;
		}
		return PlayerObject::pushButton(btn);
	}

	bool releaseButton(PlayerButton btn) {
		auto f = m_fields.self();
		if (!f->replaying && f->pending && btn == PlayerButton::Jump) {
			f->releaseQueued = true;
			return true;
		}
		return PlayerObject::releaseButton(btn);
	}

	void update(float dt) {
		PlayerObject::update(dt);
		auto f = m_fields.self();
		if (!f->pending) return;

		if (m_isOnGround) {
			f->replaying = true;
			PlayerObject::pushButton(PlayerButton::Jump);
			if (f->releaseQueued) PlayerObject::releaseButton(PlayerButton::Jump);
			f->replaying = false;
			f->pending = false;
			f->releaseQueued = false;
		} else if (--f->framesLeft <= 0) {
			f->pending = false;
			f->releaseQueued = false;
		}
	}
};
