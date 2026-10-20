#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
using namespace geode::prelude;

static bool dbg() {
	return Mod::get()->getSettingValue<bool>("debug");
}

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
		bool enabled = Mod::get()->getSettingValue<bool>("enabled");

		if (dbg()) {
			log::info("PIB press: btn={} ground={} yVel={} cube={} falling={} enabled={}",
				static_cast<int>(btn), m_isOnGround, m_yVelocity,
				isCubeMode(), isFalling(), enabled);
		}

		if (f->replaying || btn != PlayerButton::Jump || !enabled)
			return PlayerObject::pushButton(btn);

		if (isCubeMode() && !m_isOnGround && isFalling()) {
			f->pending = true;
			f->releaseQueued = false;
			f->framesLeft = Mod::get()->getSettingValue<int64_t>("window");
			if (dbg()) log::info("PIB buffered, window={}", f->framesLeft);
			return true;
		}
		if (dbg()) log::info("PIB passed through (not buffering)");
		return PlayerObject::pushButton(btn);
	}

	bool releaseButton(PlayerButton btn) {
		auto f = m_fields.self();
		if (!f->replaying && f->pending && btn == PlayerButton::Jump) {
			f->releaseQueued = true;
			if (dbg()) log::info("PIB release queued");
			return true;
		}
		return PlayerObject::releaseButton(btn);
	}

	void update(float dt) {
		PlayerObject::update(dt);
		auto f = m_fields.self();
		if (!f->pending) return;

		if (m_isOnGround) {
			if (dbg()) log::info("PIB fired on landing");
			f->replaying = true;
			PlayerObject::pushButton(PlayerButton::Jump);
			if (f->releaseQueued) PlayerObject::releaseButton(PlayerButton::Jump);
			f->replaying = false;
			f->pending = false;
			f->releaseQueued = false;
		} else if (--f->framesLeft <= 0) {
			if (dbg()) log::info("PIB window expired");
			f->pending = false;
			f->releaseQueued = false;
		}
	}
};
