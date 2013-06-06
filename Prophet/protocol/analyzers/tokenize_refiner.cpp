#include "stdafx.h"
#include "tokenize_refiner.h"


void TokenizeRefiner::RefineTree( MessageTree &tree )
{
    CalculateDepth(tree.GetRoot());
    MessageTreeRefiner::RefineTree(tree);
}

void TokenizeRefiner::RefineNode( MessageTreeNode *node )
{
    if (node->IsLeaf()) return;
    //if (m_nodeDepth[node] > m_depth) return;

//     if (node->Length() == 11 && node->GetChildrenCount() == 3) {
//         LxInfo("debug");
//     }

    std::vector<MessageTreeNode *> newChildren;
    newChildren.push_back(node->m_children[0]);
    MessageTreeNode *prev = node->m_children[0];
    for (uint i = 1; i < node->m_children.size(); i++) {
        if (m_nodeDepth[node->m_children[i]] < m_depth && 
            CanConcatenate(prev, node->m_children[i])) 
        {
            prev->m_r = node->m_children[i]->m_r;
            delete node->m_children[i];
        } else {
            prev = node->m_children[i];
            newChildren.push_back(prev);
        }
    }
    if (newChildren.size() == 1) {
        delete newChildren[0];
        node->m_children.clear();
    } else {
        node->m_children = newChildren;
    }
}

TokenizeRefiner::TokenizeRefiner( const Message *msg, MessageType type, int depth )
{
    m_msg = msg;
    m_depth = depth;
    m_type = type;
}

bool TokenizeRefiner::IsTokenChar( byte ch ) const
{
    if (m_type == MESSAGE_ASCII) {
        return !isspace(ch) && !iscntrl(ch);
    } else if (m_type == MESSAGE_BINARY) {
        //if (ch == ' ') return false;
        return (ch >= 0x20 && ch <= 0x7f) || ch == 0xa || ch == 0xd /*|| ch == 0*/;
    } else {
        LxFatal("shit\n");
        return false;
    }
}

bool TokenizeRefiner::CanConcatenate(const MessageTreeNode *l, 
                                     const MessageTreeNode *r ) const
{
//     return l->IsLeaf() && r->IsLeaf() && 
//         IsTokenChar(m_msg->Get(l->m_r)) && 
//         IsTokenChar(m_msg->Get(r->m_l));
    if (!l->IsLeaf() || !r->IsLeaf()) return false;

    if (l->m_l == l->m_r && m_msg->Get(l->m_l) == ' ')
        return false;
    if (r->m_l == r->m_r && m_msg->Get(r->m_l) == ' ')
        return false;

    for (int i = l->m_l; i <= l->m_r; i++)
        if (!IsTokenChar(m_msg->Get(i))) return false;
    for (int i = r->m_l; i <= r->m_r; i++)
        if (!IsTokenChar(m_msg->Get(i))) return false;
    return true;
}

void TokenizeRefiner::CalculateDepth( MessageTreeNode *node )
{
    if (node->IsLeaf()) {
        m_nodeDepth[node] = 0;
        return;
    }
    int d = 0;
    for (auto &c : node->m_children) {
        CalculateDepth(c);
        if (m_nodeDepth[c] > d) d = m_nodeDepth[c];
    }
    m_nodeDepth[node] = d + 1;
}
