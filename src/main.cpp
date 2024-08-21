#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include <alphalaneous.editortab_api/include/EditorTabs.hpp>

using namespace geode::prelude;

class $modify(EditorPauseLayer){

    bool init(LevelEditorLayer* p0){
		if(!EditorPauseLayer::init(p0)) return false;

		CCNode* smallActionsMenu = getChildByID("small-actions-menu");
		smallActionsMenu->getLayout()->ignoreInvisibleChildren(true);

		for(CCNode* child : CCArrayExt<CCNode*>(smallActionsMenu->getChildren())){
			child->setVisible(false);
		}

		CCNode* actionsMenu = getChildByID("actions-menu");
		actionsMenu->getLayout()->ignoreInvisibleChildren(true);

		for(CCNode* child : CCArrayExt<CCNode*>(actionsMenu->getChildren())){
			child->setVisible(false);
		}
		if (CCNode* keys = actionsMenu->getChildByID("keys-button")) {
			keys->setVisible(true);
		}

		smallActionsMenu->updateLayout();
		actionsMenu->updateLayout();

		return true;
	}
};

class $modify(MyEditorUI, EditorUI){

	struct Fields {
		Ref<EditorPauseLayer> m_pauseLayer;
	};

    bool init(LevelEditorLayer* editorLayer) {

		m_fields->m_pauseLayer = EditorPauseLayer::create(editorLayer);

		return EditorUI::init(editorLayer);
	}
};


void rebuildButtons(CCArray* arr) {


	for(CCNode* child : CCArrayExt<CCNode*>(arr)){
		child->setContentSize({40, 40});
		child->setVisible(true);
		CCSize childSize = child->getContentSize();
		
		if(ButtonSprite* buttonSprite = getChildOfType<ButtonSprite>(child, 0)) {
			buttonSprite->setContentSize({40, 40});
			buttonSprite->setScale(1);
			buttonSprite->setPosition({childSize.width/2, childSize.height/2});

			if(CCScale9Sprite* bg = getChildOfType<CCScale9Sprite>(buttonSprite, 0)) {
				bg->removeFromParent();
			}
			if(CCLabelBMFont* label = getChildOfType<CCLabelBMFont>(buttonSprite, 0)) {
				label->setScale(0.25f);
				label->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
				label->setPosition({childSize.width/2, childSize.height/2});
				std::string labelText = std::string(label->getString());
				std::replace( labelText.begin(), labelText.end(), ' ', '\n');
				label->setString(labelText.c_str());
			}
			CCSprite* buttonBG = CCSprite::create("GJ_button_04.png");
			buttonBG->setPosition({childSize.width/2, childSize.height/2});

			buttonSprite->addChild(buttonBG);
		}
	}
}

$execute {

	EditorTabs::get()->registerTab(TabType::EDIT, "build-tools"_spr, [](EditorUI* ui, CCMenuItemToggler* toggler) -> CCNode* {

        auto arr = CCArray::create();

		EditorPauseLayer* pauseLayer = static_cast<MyEditorUI*>(ui)->m_fields->m_pauseLayer;
		pauseLayer->setTouchEnabled(false);
		pauseLayer->setKeyboardEnabled(false);
		pauseLayer->setKeypadEnabled(false);

		CCNode* smallActionsMenu = pauseLayer->getChildByID("small-actions-menu");

		for(CCNode* child : CCArrayExt<CCNode*>(smallActionsMenu->getChildren())){
			arr->addObject(child);
		}
		smallActionsMenu->removeAllChildrenWithCleanup(false);

		CCNode* actionsMenu = pauseLayer->getChildByID("actions-menu");
		actionsMenu->removeChildByID("keys-button");

		for(CCNode* child : CCArrayExt<CCNode*>(actionsMenu->getChildren())){
			arr->addObject(child);
		}
		actionsMenu->removeAllChildrenWithCleanup(false);

		rebuildButtons(arr);

        CCLabelBMFont* textLabelOn = CCLabelBMFont::create("B", "bigFont.fnt");
        textLabelOn->setScale(0.4f);
        CCLabelBMFont* textLabelOff = CCLabelBMFont::create("B", "bigFont.fnt");
        textLabelOff->setScale(0.4f);

        EditorTabUtils::setTabIcons(toggler, textLabelOn, textLabelOff);

		auto winSize = cocos2d::CCDirector::get()->getWinSize();
        auto winBottom = cocos2d::CCDirector::get()->getScreenBottom();
        auto offset = cocos2d::CCPoint(winSize.width / 2 - 5.f, winBottom + ui->m_toolbarHeight / (ui->m_toolbarHeight/92) - 6.f);
        auto rows = 2;
        auto cols = 7;

		auto buttonBar = EditButtonBar::create(arr, offset, 0, false, cols, rows);
		buttonBar->setUserObject("force-rows", CCInteger::create(rows));
		buttonBar->setUserObject("force-columns", CCInteger::create(cols));

        return buttonBar;
    });
}