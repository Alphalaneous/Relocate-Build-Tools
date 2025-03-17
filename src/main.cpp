#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GJScaleControl.hpp>
#include <Geode/modify/GJTransformControl.hpp>
#include <Geode/modify/GJRotationControl.hpp>

#include <alphalaneous.editortab_api/include/EditorTabs.hpp>

using namespace geode::prelude;

class $modify(MyEditorPauseLayer, EditorPauseLayer){

	static void onModify(auto& self) {
        (void) self.setHookPriority("EditorPauseLayer::onResume", -10000); 
    }

	struct Fields {
		bool m_noResume = false;
	};

    void onResume(cocos2d::CCObject* sender) {
		if (m_fields->m_noResume) return;

		EditorPauseLayer::onResume(sender);
	}

    bool init(LevelEditorLayer* p0) {
		if (!EditorPauseLayer::init(p0)) return false;

		if (CCNode* smallActionsMenu = getChildByID("small-actions-menu")) {
			smallActionsMenu->getLayout()->ignoreInvisibleChildren(true);
			for (CCNode* child : CCArrayExt<CCNode*>(smallActionsMenu->getChildren())) {
				child->setVisible(false);
			}
			smallActionsMenu->updateLayout();
		}

		if (CCNode* actionsMenu = getChildByID("actions-menu")) {
			actionsMenu->getLayout()->ignoreInvisibleChildren(true);

			for (CCNode* child : CCArrayExt<CCNode*>(actionsMenu->getChildren())) {
				child->setVisible(false);
			}
			if (CCNode* keys = actionsMenu->getChildByID("keys-button")) {
				keys->setVisible(true);
			}
			actionsMenu->updateLayout();
		}

		return true;
	}
};

std::map<std::string, std::string> labelToIcon = {
	{"Reset\nScroll", "ResetScroll"},
	{"Create\nLoop", "CreateLoop"},
	{"Re-\nGroup", "Regroup"},
	{"AlignX", "AlignX"},
	{"AlignY", "AlignY"},
	{"Select\nAll", "SelectAll"},
	{"Select\nAll\nLeft", "SelectAllLeft"},
	{"Select\nAll\nRight", "SelectAllRight"},
	{"New\nGroupX", "CreateGroupX"},
	{"New\nGroupY", "CreateGroupY"},
	{"Build\nHelper", "BuildHelper"},
	{"Copy+\nColor", "CopyColor"},
	{"Paste+\nColor", "PasteColor"},
	{"Unlock\nLayers", "UnlockLayers"},
	{"Reset\nUnused", "ResetUnused"},
	{"Uncheck\nPortals", "UncheckPortals"},
};

CCSprite* addIcon(CCNode* node, CCLabelBMFont* label) {

	std::string labelText = std::string(label->getString());

	std::string iconTexture;

	bool iconsOnly = false;

	if (Mod::get()->getSettingValue<std::string>("icons") == "Icons Only") {
		iconTexture = fmt::format("o_{}.png"_spr, labelToIcon[labelText]);
		iconsOnly = true;
	}
	else if (Mod::get()->getSettingValue<std::string>("icons") == "Icons & Text"){
		iconTexture = fmt::format("u_{}.png"_spr, labelToIcon[labelText]);
	}

	if (CCSprite* spr = CCSprite::create(iconTexture.c_str())){
		spr->setZOrder(1);
		if (!iconsOnly){
			spr->setColor({0, 0, 0});
			spr->setOpacity(96);
		}
		spr->setScale(0.7f);
		spr->setPosition({node->getContentSize().width/2, node->getContentSize().height/2});
		if (spr->getUserObject("geode.texture-loader/fallback")) return nullptr;
		node->addChild(spr);
		return spr;
	}
	return nullptr;
}

void rebuildButtons(CCArray* arr) {

	for (CCNode* child : CCArrayExt<CCNode*>(arr)) {
		child->setContentSize({40, 40});
		child->setVisible(true);
		CCSize childSize = child->getContentSize();
		
		if (ButtonSprite* buttonSprite = child->getChildByType<ButtonSprite>(0)) {
			buttonSprite->setContentSize({40, 40});
			buttonSprite->setScale(1);
			buttonSprite->setPosition({childSize.width/2, childSize.height/2});

			if (CCScale9Sprite* bg = buttonSprite->getChildByType<CCScale9Sprite>(0)) {
				bg->removeFromParent();
			}
			if (CCLabelBMFont* label = buttonSprite->getChildByType<CCLabelBMFont>(0)) {
				label->setScale(0.25f);
				label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
				label->setPosition({childSize.width/2, childSize.height/2});
				std::string labelText = std::string(label->getString());
				std::replace(labelText.begin(), labelText.end(), ' ', '\n');
				label->setString(labelText.c_str());
				label->setZOrder(2);
				if (Mod::get()->getSettingValue<std::string>("icons") != "None"){
					if (addIcon(buttonSprite, label) && Mod::get()->getSettingValue<std::string>("icons") == "Icons Only") {
						label->setVisible(false);
					}
				}
			}
			CCSprite* buttonBG = CCSprite::create("GJ_button_04.png");
			buttonBG->setPosition({childSize.width/2, childSize.height/2});

			buttonSprite->addChild(buttonBG);
		}
	}
}

class $modify(MyEditorUI, EditorUI) {

	struct Fields {
		Ref<EditorPauseLayer> m_pauseLayer;
	};

    bool init(LevelEditorLayer* editorLayer) {

		auto fields = m_fields.self();

		fields->m_pauseLayer = EditorPauseLayer::create(editorLayer);
		fields->m_pauseLayer->setTouchEnabled(false);
		fields->m_pauseLayer->setKeyboardEnabled(false);
		fields->m_pauseLayer->setKeypadEnabled(false);
		static_cast<MyEditorPauseLayer*>(fields->m_pauseLayer.data())->m_fields->m_noResume = true;

		CCTouchDispatcher::get()->unregisterForcePrio(fields->m_pauseLayer);
		CCTouchDispatcher::get()->removeDelegate(fields->m_pauseLayer);

		queueInMainThread([this] {
			if (auto delegate = typeinfo_cast<CCTouchDelegate*>(m_fields->m_pauseLayer.data())) {
				if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
					CCTouchDispatcher::get()->setPriority(100000, handler->getDelegate());
				}
			}
		});

		if (!EditorUI::init(editorLayer)) return false;

		EditorTabs::get()->addTab(this, TabType::EDIT, "build-tools"_spr, create_tab_callback(MyEditorUI::createBuildTools));

		return true;
	}

	CCNode* createBuildTools(EditorUI* ui, CCMenuItemToggler* toggler) {
		auto arr = CCArray::create();

		EditorPauseLayer* pauseLayer = static_cast<MyEditorUI*>(ui)->m_fields->m_pauseLayer;
		
		if (CCNode* smallActionsMenu = pauseLayer->getChildByID("small-actions-menu")) {
			for (CCNode* child : CCArrayExt<CCNode*>(smallActionsMenu->getChildren())) {
				arr->addObject(child);
			}
			smallActionsMenu->removeAllChildrenWithCleanup(false);
		}

		if (CCNode* actionsMenu = pauseLayer->getChildByID("actions-menu")) {
			actionsMenu->removeChildByID("keys-button");

			for (CCNode* child : CCArrayExt<CCNode*>(actionsMenu->getChildren())) {
				arr->addObject(child);
			}

			actionsMenu->removeAllChildrenWithCleanup(false);
		}

		if (arr->count() > 0) {
			rebuildButtons(arr);
		}

		CCLabelBMFont* textLabelOn = CCLabelBMFont::create("B", "bigFont.fnt");
		textLabelOn->setScale(0.4f);
		CCLabelBMFont* textLabelOff = CCLabelBMFont::create("B", "bigFont.fnt");
		textLabelOff->setScale(0.4f);

		EditorTabUtils::setTabIcons(toggler, textLabelOn, textLabelOff);

		auto winSize = cocos2d::CCDirector::get()->getWinSize();
		auto winBottom = cocos2d::CCDirector::get()->getScreenBottom();
		auto offset = cocos2d::CCPoint(winSize.width / 2 - 5.f, winBottom + 92 - 6.f);

		EditButtonBar* buttonBar;
		if (Mod::get()->getSettingValue<bool>("force-button-size")) {
			int rows = 2;
			int cols = 7;

			buttonBar = EditButtonBar::create(arr, offset, 0, false, cols, rows);
			buttonBar->setUserObject("force-rows", CCInteger::create(rows));
			buttonBar->setUserObject("force-columns", CCInteger::create(cols));
		}
		else {
			auto rows = GameManager::get()->getIntGameVariable("0050");
			auto cols = GameManager::get()->getIntGameVariable("0049");
			buttonBar = EditButtonBar::create(arr, offset, 0, false, cols, rows);
		}

		return buttonBar;
	}
};

