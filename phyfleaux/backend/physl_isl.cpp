//  Copyright (c) 2019-2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <isl/aff.h>
#include <isl/set.h>
#include <isl/map.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <isl/id.h>
#include <isl/val.h>
#include <isl/ast_type.h>

// this is required and can only be found in
// the source tree for isl
//
// this file is not installed as part of
// the isl build
//
#include "isl_ast_private.h"

#include "physl_isl.hpp"
#include <string>
#include <cstring>
//#include <algorithm>

#define isl_ast_op_last	isl_ast_op_address_of

namespace physl { namespace codegen {

__isl_give isl_printer *isl_ast_node_list_print__(
        __isl_keep isl_ast_node_list *list, __isl_take isl_printer *p,
        __isl_keep isl_ast_print_options *options);

/* Print the start of a compound statement.
 */
static __isl_give isl_printer *start_block(__isl_take isl_printer *p)
{
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, "block(");
	p = isl_printer_end_line(p);
	p = isl_printer_indent(p, 2);

	return p;
}

/* Print the end of a compound statement.
 */
static __isl_give isl_printer *end_block(__isl_take isl_printer *p)
{
	p = isl_printer_indent(p, -2);
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, ")");
	p = isl_printer_end_line(p);

	return p;
}

static int need_block(__isl_keep isl_ast_node *node)
{
	isl_ctx *ctx;

	if (node->type == isl_ast_node_block)
		return 1;
	if (node->type == isl_ast_node_for && node->u.f.degenerate)
		return 1;
	if (node->type == isl_ast_node_if && node->u.i.else_node)
		return 1;
	if (node->type == isl_ast_node_mark)
		return 1;

	ctx = isl_ast_node_get_ctx(node);
	return isl_options_get_ast_always_print_block(ctx);
}

/* Retrieve the note identified by "id" from "p".
 * The note is assumed to exist.
 */
static void *get_note(__isl_keep isl_printer *p, __isl_keep isl_id *id)
{
	void *note;

	id = isl_printer_get_note(p, isl_id_copy(id));
	note = isl_id_get_user(id);
	isl_id_free(id);

	return note;
}

/* Return the node marked by mark node "node".
 */
__isl_give isl_ast_node *isl_ast_node_mark_get_node(
	__isl_keep isl_ast_node *node)
{
	if (!node)
		return NULL;
	if (node->type != isl_ast_node_mark)
		isl_die(isl_ast_node_get_ctx(node), isl_error_invalid,
			"not a mark node", return NULL);

	return isl_ast_node_copy(node->u.m.node);
}

/* Data structure that holds the user-specified textual
 * representations for the operators in C format.
 * The entries are either NULL or copies of strings.
 * A NULL entry means that the default name should be used.
 */
struct isl_ast_op_names {
	char *op_str[isl_ast_op_last + 1];
};

/* Create an empty struct isl_ast_op_names.
 */
static void *create_names(isl_ctx *ctx)
{
	return isl_calloc_type(ctx, struct isl_ast_op_names);
}

/* Free a struct isl_ast_op_names along with all memory
 * owned by the struct.
 */
static void free_names(void *user)
{
	int i;
	struct isl_ast_op_names *names = static_cast<struct isl_ast_op_names *>(user);

	if (!user)
		return;

	for (i = 0; i <= isl_ast_op_last; ++i)
		free(names->op_str[i]);
	free(user);
}

/* Create an identifier that is used to store
 * an isl_ast_op_names note.
 */
static __isl_give isl_id *names_id(isl_ctx *ctx)
{
	return isl_id_alloc(ctx, "isl_ast_op_type_names", NULL);
}



static __isl_give isl_printer *print_ast_node_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_node *node,
	__isl_keep isl_ast_print_options *options, int in_block, int in_list);

static __isl_give isl_printer *isl_printer_print_ast_expr_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_expr *expr);

/* Print the body "node" of a for or if node.
 * If "else_node" is set, then it is printed as well.
 * If "force_block" is set, then print out the body as a block.
 * We first check if we need to print out a block.
 * We always print out a block if there is an else node to make
 * sure that the else node is matched to the correct if node.
 * For consistency, the corresponding else node is also printed as a block.
 *
 * If the else node is itself an if, then we print it as
 *
 *	} else if (..) {
 *	}
 *
 * Otherwise the else node is printed as
 *
 *	} else {
 *	  node
 *	}
 */
static __isl_give isl_printer *print_body_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_node *node, __isl_keep isl_ast_node *else_node,
	__isl_keep isl_ast_print_options *options, int force_block)
{
	if (!node)
		return isl_printer_free(p);

	if (!force_block && !else_node && !need_block(node)) {
		p = isl_printer_end_line(p);
		p = isl_printer_indent(p, 2);
                // TODO
                {
                    isl_ast_print_options * opt = isl_ast_print_options_copy(options);
                    if (!options || !node)
                        goto error;
                    p = print_ast_node_physl(p, node, options, 0, 0);
                    isl_ast_print_options_free(options);
                    return p;

error:
                    isl_ast_print_options_free(options);
                    isl_printer_free(p);
                    return nullptr;
                }
		//p = isl_ast_node_print(node, p,
		//			isl_ast_print_options_copy(options));
		p = isl_printer_indent(p, -2);
		return p;
	}

	p = isl_printer_print_str(p, " block( ");
	p = isl_printer_end_line(p);
	p = isl_printer_indent(p, 2);
	p = print_ast_node_physl(p, node, options, 1, 0);
	p = isl_printer_indent(p, -2);
	p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, " ) ");
	if (else_node) {
                // ct note: this commented out section provides 
                //          support for 'else if' expressions 
                //
		//if (else_node->type == isl_ast_node_if) {
		//	p = isl_printer_print_str(p, " else ");
		//	p = print_if_physl(p, else_node, options, 0, 1);
                //
                // ct note: this section provides support for
                //          'else' expressions
		//} else {
			p = isl_printer_print_str(p, " , ");
			p = print_body_physl(p, else_node, NULL, options, 1);
	                p = isl_printer_print_str(p, " ) ");
		//}
	} else
		p = isl_printer_end_line(p);

	return p;
}

static __isl_give isl_printer *print_for_physl(
    __isl_take isl_printer *p,
    __isl_keep isl_ast_node *node,
    __isl_keep isl_ast_print_options *options, int in_block, int in_list)
{
	isl_id *id;
	const char *name;
	const char *type;

	type = isl_options_get_ast_iterator_type(isl_printer_get_ctx(p));
	if (!node->u.f.degenerate) {
		id = isl_ast_expr_get_id(node->u.f.iterator);
		name = isl_id_get_name(id);
		isl_id_free(id);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "for( ");
		p = isl_printer_print_str(p, " store( ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, ", ");
		p = isl_printer_print_ast_expr_physl(p, node->u.f.init);
		p = isl_printer_print_str(p, " ), ");
		p = isl_printer_print_ast_expr_physl(p, node->u.f.cond);
		p = isl_printer_print_str(p, ", ");
		p = isl_printer_print_str(p, " store( ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, " , ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, " + ");
		p = isl_printer_print_ast_expr_physl(p, node->u.f.inc);
		p = isl_printer_print_str(p, "), ");
		p = isl_printer_print_str(p, " block( ");
		p = print_body_physl(p, node->u.f.body, NULL, options, 0);
		p = isl_printer_print_str(p, " )"); // end block
		p = isl_printer_print_str(p, " )"); // end for
	} else {
		id = isl_ast_expr_get_id(node->u.f.iterator);
		name = isl_id_get_name(id);
		isl_id_free(id);
		if (!in_block || in_list)
			p = start_block(p);
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "store( ");
		p = isl_printer_print_str(p, name);
		p = isl_printer_print_str(p, " , ");
		p = isl_printer_print_ast_expr_physl(p, node->u.f.init);
		p = isl_printer_print_str(p, ")");
		p = isl_printer_end_line(p);
		p = print_ast_node_physl(p, node->u.f.body, options, 1, 0);
		if (!in_block || in_list)
			p = end_block(p);
	}

	return p;
}

/* Print the if node "node".
 * If "new_line" is set then the if node should be printed on a new line.
 * If "force_block" is set, then print out the body as a block.
 */
static __isl_give isl_printer *print_if_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_node *node,
	__isl_keep isl_ast_print_options *options, int new_line,
	int force_block)
{
	if (new_line)
		p = isl_printer_start_line(p);
	p = isl_printer_print_str(p, "if (");
	p = isl_printer_print_ast_expr_physl(p, node->u.i.guard);
	p = isl_printer_print_str(p, " , ");
	p = print_body_physl(p, node->u.i.then, node->u.i.else_node, options,
			force_block);
	p = isl_printer_print_str(p, " )");

	return p;
}

/* Return a string containing PhySL code representing this isl_ast_node.
__isl_give char *isl_ast_node_to_physl_str(__isl_keep isl_ast_node *node)
{
	isl_printer *p;
	char *str;

	if (!node)
	    return nullptr;

	p = isl_printer_to_str(isl_ast_node_get_ctx(node));
	p = isl_printer_print_ast_node(p, node);

	str = isl_printer_get_str(p);

	isl_printer_free(p);

	return str;
}
 */


/* Print a function call "expr" in C format.
 *
 * The first argument represents the function to be called.
 */
static __isl_give isl_printer *print_call_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_expr *expr)
{
	int i = 0;

	p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[0]);
	p = isl_printer_print_str(p, "(");
	for (i = 1; i < expr->u.op.n_arg; ++i) {
		if (i != 1)
			p = isl_printer_print_str(p, ", ");
		p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[i]);
	}
	p = isl_printer_print_str(p, ")");

	return p;
}

/* Print an array access "expr" in C format.
 *
 * The first argument represents the array being accessed.
 */
static __isl_give isl_printer *print_access_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_expr *expr)
{
	int i = 0;

	p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[0]);
	for (i = 1; i < expr->u.op.n_arg; ++i) {
		p = isl_printer_print_str(p, "[");
		p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[i]);
		p = isl_printer_print_str(p, "]");
	}

	return p;
}

/* Textual C representation of the various operators.
 */
static std::string op_str_c [] = {
	[isl_ast_op_and] = "&&",
	[isl_ast_op_and_then] = "&&",
	[isl_ast_op_or] = "||",
	[isl_ast_op_or_else] = "||",
	[isl_ast_op_max] = "max",
	[isl_ast_op_min] = "min",
	[isl_ast_op_minus] = "-",
	[isl_ast_op_add] = "+",
	[isl_ast_op_sub] = "-",
	[isl_ast_op_mul] = "*",
	[isl_ast_op_fdiv_q] = "floord",
	[isl_ast_op_pdiv_q] = "/",
	[isl_ast_op_pdiv_r] = "%",
	[isl_ast_op_zdiv_r] = "%",
	[isl_ast_op_div] = "/",
	[isl_ast_op_eq] = "==",
	[isl_ast_op_le] = "<=",
	[isl_ast_op_ge] = ">=",
	[isl_ast_op_lt] = "<",
	[isl_ast_op_gt] = ">",
	[isl_ast_op_member] = ".",
	[isl_ast_op_address_of] = "&"
};

/* Precedence in C of the various operators.
 * Based on http://en.wikipedia.org/wiki/Operators_in_C_and_C++
 * Lowest value means highest precedence.
 */
static int op_prec[] = {
	[isl_ast_op_and] = 13,
	[isl_ast_op_and_then] = 13,
	[isl_ast_op_or] = 14,
	[isl_ast_op_or_else] = 14,
	[isl_ast_op_max] = 2,
	[isl_ast_op_min] = 2,
	[isl_ast_op_minus] = 3,
	[isl_ast_op_add] = 6,
	[isl_ast_op_sub] = 6,
	[isl_ast_op_mul] = 5,
	[isl_ast_op_div] = 5,
	[isl_ast_op_fdiv_q] = 2,
	[isl_ast_op_pdiv_q] = 5,
	[isl_ast_op_pdiv_r] = 5,
	[isl_ast_op_zdiv_r] = 5,
	[isl_ast_op_cond] = 15,
	[isl_ast_op_select] = 15,
	[isl_ast_op_eq] = 9,
	[isl_ast_op_le] = 8,
	[isl_ast_op_ge] = 8,
	[isl_ast_op_lt] = 8,
	[isl_ast_op_gt] = 8,
	[isl_ast_op_call] = 2,
	[isl_ast_op_access] = 2,
	[isl_ast_op_member] = 2,
	[isl_ast_op_address_of] = 3
};

/* Is the operator left-to-right associative?
 */
static int op_left[] = {
	[isl_ast_op_and] = 1,
	[isl_ast_op_and_then] = 1,
	[isl_ast_op_or] = 1,
	[isl_ast_op_or_else] = 1,
	[isl_ast_op_max] = 1,
	[isl_ast_op_min] = 1,
	[isl_ast_op_minus] = 0,
	[isl_ast_op_add] = 1,
	[isl_ast_op_sub] = 1,
	[isl_ast_op_mul] = 1,
	[isl_ast_op_div] = 1,
	[isl_ast_op_fdiv_q] = 1,
	[isl_ast_op_pdiv_q] = 1,
	[isl_ast_op_pdiv_r] = 1,
	[isl_ast_op_zdiv_r] = 1,
	[isl_ast_op_cond] = 0,
	[isl_ast_op_select] = 0,
	[isl_ast_op_eq] = 1,
	[isl_ast_op_le] = 1,
	[isl_ast_op_ge] = 1,
	[isl_ast_op_lt] = 1,
	[isl_ast_op_gt] = 1,
	[isl_ast_op_call] = 1,
	[isl_ast_op_access] = 1,
	[isl_ast_op_member] = 1,
	[isl_ast_op_address_of] = 0
};

static int is_and(enum isl_ast_op_type op)
{
	return op == isl_ast_op_and || op == isl_ast_op_and_then;
}

static int is_or(enum isl_ast_op_type op)
{
	return op == isl_ast_op_or || op == isl_ast_op_or_else;
}

static int is_add_sub(enum isl_ast_op_type op)
{
	return op == isl_ast_op_add || op == isl_ast_op_sub;
}

static int is_div_mod(enum isl_ast_op_type op)
{
	return op == isl_ast_op_div ||
	       op == isl_ast_op_pdiv_r ||
	       op == isl_ast_op_zdiv_r;
}

/* Do we need/want parentheses around "expr" as a subexpression of
 * an "op" operation?  If "left" is set, then "expr" is the left-most
 * operand.
 *
 * We only need parentheses if "expr" represents an operation.
 *
 * If op has a higher precedence than expr->u.op.op, then we need
 * parentheses.
 * If op and expr->u.op.op have the same precedence, but the operations
 * are performed in an order that is different from the associativity,
 * then we need parentheses.
 *
 * An and inside an or technically does not require parentheses,
 * but some compilers complain about that, so we add them anyway.
 *
 * Computations such as "a / b * c" and "a % b + c" can be somewhat
 * difficult to read, so we add parentheses for those as well.
 */
static int sub_expr_need_parens(enum isl_ast_op_type op,
	__isl_keep isl_ast_expr *expr, int left)
{
	if (expr->type != isl_ast_expr_op)
		return 0;

	if (op_prec[expr->u.op.op] > op_prec[op])
		return 1;
	if (op_prec[expr->u.op.op] == op_prec[op] && left != op_left[op])
		return 1;

	if (is_or(op) && is_and(expr->u.op.op))
		return 1;
	if (op == isl_ast_op_mul && expr->u.op.op != isl_ast_op_mul &&
	    op_prec[expr->u.op.op] == op_prec[op])
		return 1;
	if (is_add_sub(op) && is_div_mod(expr->u.op.op))
		return 1;

	return 0;
}

/* Print "expr" as a subexpression of an "op" operation in C format.
 * If "left" is set, then "expr" is the left-most operand.
 */
static __isl_give isl_printer *print_sub_expr_physl(__isl_take isl_printer *p,
	enum isl_ast_op_type op, __isl_keep isl_ast_expr *expr, int left)
{
	int need_parens;

	need_parens = sub_expr_need_parens(op, expr, left);

	if (need_parens)
		p = isl_printer_print_str(p, "(");
	p = isl_printer_print_ast_expr_physl(p, expr);
	if (need_parens)
		p = isl_printer_print_str(p, ")");
	return p;
}

/* Return the textual representation of "type" in C format.
 *
 * If there is a user-specified name in an isl_ast_op_names note
 * associated to "p", then return that.
 * Otherwise, return the default name in op_str_c.
 */
static const char *get_op_str_physl(__isl_keep isl_printer *p,
	enum isl_ast_op_type type)
{
	isl_id *id;
	isl_bool has_names;
	struct isl_ast_op_names *names = NULL;

	id = names_id(isl_printer_get_ctx(p));
	has_names = isl_printer_has_note(p, id);
	if (has_names >= 0 && has_names)
		names = static_cast<struct isl_ast_op_names *>(get_note(p, id));
	isl_id_free(id);
	if (names && names->op_str[type])
		return names->op_str[type];
	return op_str_c[type].c_str();
}

/* Print a min or max reduction "expr" in C format.
 */
static __isl_give isl_printer *print_min_max_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_expr *expr)
{
	int i = 0;

	for (i = 1; i < expr->u.op.n_arg; ++i) {
		p = isl_printer_print_str(p, get_op_str_physl(p, expr->u.op.op));
		p = isl_printer_print_str(p, "(");
	}
	p = isl_printer_print_ast_expr(p, expr->u.op.args[0]);
	for (i = 1; i < expr->u.op.n_arg; ++i) {
		p = isl_printer_print_str(p, ", ");
		p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[i]);
		p = isl_printer_print_str(p, ")");
	}

	return p;
}

static __isl_give isl_printer *isl_printer_print_ast_expr_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_expr *expr)
{
	if (!p)
		return NULL;
	if (!expr)
		return isl_printer_free(p);

	switch (expr->type) {
	case isl_ast_expr_op:
		if (expr->u.op.op == isl_ast_op_call) {
			p = print_call_physl(p, expr);
			break;
		}
		if (expr->u.op.op == isl_ast_op_access) {
			p = print_access_physl(p, expr);
			break;
		}
		if (expr->u.op.n_arg == 1) {
			p = isl_printer_print_str(p,
						get_op_str_physl(p, expr->u.op.op));
			p = print_sub_expr_physl(p, expr->u.op.op,
						expr->u.op.args[0], 0);
			break;
		}
		if (expr->u.op.op == isl_ast_op_fdiv_q) {
			const char *name;

			name = get_op_str_physl(p, isl_ast_op_fdiv_q);
			p = isl_printer_print_str(p, name);
			p = isl_printer_print_str(p, "(");
			p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[0]);
			p = isl_printer_print_str(p, ", ");
			p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[1]);
			p = isl_printer_print_str(p, ")");
			break;
		}
		if (expr->u.op.op == isl_ast_op_max ||
		    expr->u.op.op == isl_ast_op_min) {
			p = print_min_max_physl(p, expr);
			break;
		}
		if (expr->u.op.op == isl_ast_op_cond ||
		    expr->u.op.op == isl_ast_op_select) {
			p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[0]);
			p = isl_printer_print_str(p, " ? ");
			p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[1]);
			p = isl_printer_print_str(p, " : ");
			p = isl_printer_print_ast_expr_physl(p, expr->u.op.args[2]);
			break;
		}
		if (expr->u.op.n_arg != 2)
			isl_die(isl_printer_get_ctx(p), isl_error_internal,
				"operation should have two arguments",
				return isl_printer_free(p));
		p = print_sub_expr_physl(p, expr->u.op.op, expr->u.op.args[0], 1);
		if (expr->u.op.op != isl_ast_op_member)
			p = isl_printer_print_str(p, " ");
		p = isl_printer_print_str(p, get_op_str_physl(p, expr->u.op.op));
		if (expr->u.op.op != isl_ast_op_member)
			p = isl_printer_print_str(p, " ");
		p = print_sub_expr_physl(p, expr->u.op.op, expr->u.op.args[1], 0);
		break;
	case isl_ast_expr_id:
		p = isl_printer_print_str(p, isl_id_get_name(expr->u.id));
		break;
	case isl_ast_expr_int:
		p = isl_printer_print_val(p, expr->u.v);
		break;
	case isl_ast_expr_error:
		break;
	}

	return p;
}

/* Print the "node" to "p".
 *
 * "in_block" is set if we are currently inside a block.
 * If so, we do not print a block around the children of a block node.
 * We do this to avoid an extra block around the body of a degenerate
 * for node.
 *
 * "in_list" is set if the current node is not alone in the block.
 */
static __isl_give isl_printer *print_ast_node_physl(__isl_take isl_printer *p,
	__isl_keep isl_ast_node *node,
	__isl_keep isl_ast_print_options *options, int in_block, int in_list)
{
	switch (node->type) {
	case isl_ast_node_for:
		if (options->print_for) {
			return options->print_for(p,
					isl_ast_print_options_copy(options),
					node, options->print_for_user);
                }
		p = print_for_physl(p, node, options, in_block, in_list);
		break;
	case isl_ast_node_if:
		p = print_if_physl(p, node, options, 1, 0);
		break;
	case isl_ast_node_block:
		if (!in_block)
			p = start_block(p);
		p = isl_ast_node_list_print__(node->u.b.children, p, options);
		if (!in_block)
			p = end_block(p);
		break;
	case isl_ast_node_mark:
		p = isl_printer_start_line(p);
		p = isl_printer_print_str(p, "// ");
		p = isl_printer_print_str(p, isl_id_get_name(node->u.m.mark));
		p = isl_printer_end_line(p);
		p = print_ast_node_physl(p, node->u.m.node, options, 0, in_list);
		break;
	case isl_ast_node_user:
		if (options->print_user) {
			return options->print_user(p,
					isl_ast_print_options_copy(options),
					node, options->print_user_user);
                }
		p = isl_printer_start_line(p);
		p = isl_printer_print_ast_expr_physl(p, node->u.e.expr);
		//p = isl_printer_print_str(p, ")");
		p = isl_printer_end_line(p);
		break;
	case isl_ast_node_error:
		break;
	}
	return p;
}

static __isl_give char *isl_ast_node_to_physl_str(isl_ctx* ctx, isl_ast_node* node)
{
	isl_printer *p = nullptr;
	char *str = nullptr;

	if (!node) {
	    return nullptr;
        }

	p = isl_printer_to_str(ctx); //isl_ast_node_get_ctx(node));

        isl_ast_print_options *options = isl_ast_print_options_alloc(isl_printer_get_ctx(p));
	p = print_ast_node_physl(p, node, options, 0, 0);
	str = isl_printer_get_str(p);

        isl_ast_print_options_free(options);
	isl_printer_free(p);

	return str;
}

__isl_give isl_printer *isl_ast_node_list_print__(
        __isl_keep isl_ast_node_list *list, __isl_take isl_printer *p,
        __isl_keep isl_ast_print_options *options)
{
        int i;

        if (!p || !list || !options)
                return isl_printer_free(p);

        for (i = 0; i < list->n; ++i)
                p = print_ast_node_physl(p, list->p[i], options, 1, 1);

        return p;
}

int generate_physl(isl_ctx * ctx, isl_ast_node * node) {
    isl_printer *p = isl_printer_to_file(ctx, stdout); //isl_ast_node_get_ctx(node.get()), stdout);
    isl_ast_print_options *options = isl_ast_print_options_alloc(isl_printer_get_ctx(p));

    p = print_ast_node_physl(p, node, options, 0, 0);

    isl_ast_print_options_free(options);
    isl_printer_free(p);

    return 1;
}


int generate_physl(isl_ctx * ctx, isl_ast_node * node, std::ostream & fstr) {
    char *c_str = isl_ast_node_to_physl_str(ctx, node);
    std::string cstr{(c_str == nullptr) ? "" : c_str };
    fstr << cstr; //(cstr.c_str(), cstr.size());

    return 1;
}

int generate_physl(isl_ctx * ctx, isl_ast_node * node, std::string & physlstr) {
    char * c_str = isl_ast_node_to_physl_str(ctx, node);
    std::string cstr{(c_str == nullptr) ? "" : c_str };
    physlstr.assign(cstr);

    return 1;
}

} /* end namespace codegen */ } // end namespace physl
