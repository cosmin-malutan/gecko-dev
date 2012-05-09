/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include "nsIDOMHTMLOptGroupElement.h"
#include "nsIDOMEventTarget.h"
#include "nsGenericHTMLElement.h"
#include "nsGkAtoms.h"
#include "nsStyleConsts.h"
#include "nsIFrame.h"
#include "nsIFormControlFrame.h"
#include "nsEventStates.h"
#include "nsIDocument.h"

#include "nsEventDispatcher.h"
#include "nsHTMLSelectElement.h"

/**
 * The implementation of &lt;optgroup&gt;
 */
class nsHTMLOptGroupElement : public nsGenericHTMLElement,
                              public nsIDOMHTMLOptGroupElement
{
public:
  nsHTMLOptGroupElement(already_AddRefed<nsINodeInfo> aNodeInfo);
  virtual ~nsHTMLOptGroupElement();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_FORWARD_NSIDOMNODE(nsGenericHTMLElement::)

  // nsIDOMElement
  NS_FORWARD_NSIDOMELEMENT(nsGenericHTMLElement::)

  // nsIDOMHTMLElement
  NS_FORWARD_NSIDOMHTMLELEMENT(nsGenericHTMLElement::)

  // nsIDOMHTMLOptGroupElement
  NS_DECL_NSIDOMHTMLOPTGROUPELEMENT

  // nsINode
  virtual nsresult InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                 bool aNotify);
  virtual void RemoveChildAt(PRUint32 aIndex, bool aNotify);

  // nsIContent
  virtual nsresult PreHandleEvent(nsEventChainPreVisitor& aVisitor);

  virtual nsEventStates IntrinsicState() const;
 
  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;

  virtual nsXPCClassInfo* GetClassInfo();

  virtual nsIDOMNode* AsDOMNode() { return this; }

  virtual bool IsDisabled() const {
    return HasAttr(kNameSpaceID_None, nsGkAtoms::disabled);
  }
protected:

  /**
   * Get the select content element that contains this option
   * @param aSelectElement the select element [OUT]
   */
  nsIContent* GetSelect();
};


NS_IMPL_NS_NEW_HTML_ELEMENT(OptGroup)


nsHTMLOptGroupElement::nsHTMLOptGroupElement(already_AddRefed<nsINodeInfo> aNodeInfo)
  : nsGenericHTMLElement(aNodeInfo)
{
  // We start off enabled
  AddStatesSilently(NS_EVENT_STATE_ENABLED);
}

nsHTMLOptGroupElement::~nsHTMLOptGroupElement()
{
}


NS_IMPL_ADDREF_INHERITED(nsHTMLOptGroupElement, nsGenericElement)
NS_IMPL_RELEASE_INHERITED(nsHTMLOptGroupElement, nsGenericElement)


DOMCI_NODE_DATA(HTMLOptGroupElement, nsHTMLOptGroupElement)

// QueryInterface implementation for nsHTMLOptGroupElement
NS_INTERFACE_TABLE_HEAD(nsHTMLOptGroupElement)
  NS_HTML_CONTENT_INTERFACE_TABLE1(nsHTMLOptGroupElement,
                                   nsIDOMHTMLOptGroupElement)
  NS_HTML_CONTENT_INTERFACE_TABLE_TO_MAP_SEGUE(nsHTMLOptGroupElement,
                                               nsGenericHTMLElement)
NS_HTML_CONTENT_INTERFACE_TABLE_TAIL_CLASSINFO(HTMLOptGroupElement)


NS_IMPL_ELEMENT_CLONE(nsHTMLOptGroupElement)


NS_IMPL_BOOL_ATTR(nsHTMLOptGroupElement, Disabled, disabled)
NS_IMPL_STRING_ATTR(nsHTMLOptGroupElement, Label, label)


nsresult
nsHTMLOptGroupElement::PreHandleEvent(nsEventChainPreVisitor& aVisitor)
{
  aVisitor.mCanHandle = false;
  // Do not process any DOM events if the element is disabled
  // XXXsmaug This is not the right thing to do. But what is?
  if (HasAttr(kNameSpaceID_None, nsGkAtoms::disabled)) {
    return NS_OK;
  }

  nsIFrame* frame = GetPrimaryFrame();
  if (frame) {
    const nsStyleUserInterface* uiStyle = frame->GetStyleUserInterface();
    if (uiStyle->mUserInput == NS_STYLE_USER_INPUT_NONE ||
        uiStyle->mUserInput == NS_STYLE_USER_INPUT_DISABLED) {
      return NS_OK;
    }
  }

  return nsGenericHTMLElement::PreHandleEvent(aVisitor);
}

nsIContent*
nsHTMLOptGroupElement::GetSelect()
{
  nsIContent* parent = this;
  while ((parent = parent->GetParent()) && parent->IsHTML()) {
    if (parent->Tag() == nsGkAtoms::select) {
      return parent;
    }
    if (parent->Tag() != nsGkAtoms::optgroup) {
      break;
    }
  }
  
  return nsnull;
}

nsresult
nsHTMLOptGroupElement::InsertChildAt(nsIContent* aKid,
                                     PRUint32 aIndex,
                                     bool aNotify)
{
  nsSafeOptionListMutation safeMutation(GetSelect(), this, aKid, aIndex, aNotify);
  nsresult rv = nsGenericHTMLElement::InsertChildAt(aKid, aIndex, aNotify);
  if (NS_FAILED(rv)) {
    safeMutation.MutationFailed();
  }
  return rv;
}

void
nsHTMLOptGroupElement::RemoveChildAt(PRUint32 aIndex, bool aNotify)
{
  nsSafeOptionListMutation safeMutation(GetSelect(), this, nsnull, aIndex,
                                        aNotify);
  nsGenericHTMLElement::RemoveChildAt(aIndex, aNotify);
}

nsEventStates
nsHTMLOptGroupElement::IntrinsicState() const
{
  nsEventStates state = nsGenericHTMLElement::IntrinsicState();

  if (HasAttr(kNameSpaceID_None, nsGkAtoms::disabled)) {
    state |= NS_EVENT_STATE_DISABLED;
    state &= ~NS_EVENT_STATE_ENABLED;
  } else {
    state &= ~NS_EVENT_STATE_DISABLED;
    state |= NS_EVENT_STATE_ENABLED;
  }

  return state;
}
